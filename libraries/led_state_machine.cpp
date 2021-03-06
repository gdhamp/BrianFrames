/**
* @file led_state_machine.cpp
* @brief implements the Easing and LedStateMachine objects
*
* Copyright (c) 2016 Heads Up Display, Inc
*/
#include "defines.h"
#include "led_state_machine.h"

extern DigitalOut g_DisplayPower;

/**
* Create the LedStateMachine object, and reset the m_MessageQueue
*
* @param [in] a_SpiLeds - a TLC59711 shared between this object and others
* @param [in] a_MessageQueue - a PacketQueue shared between this object and others
*/
LedStateMachine::LedStateMachine(TLC59711& a_SpiLeds, PacketQueue& a_MessageQueue) : m_SpiLeds(a_SpiLeds), m_MessageQueue(a_MessageQueue), m_CurrentGroupId(0xff), m_DismissGroup(false)
{
	// Note - RgbLeds are clear by their constructor
	reset();
}

/**
* Reset all member variables, including resetting the m_MessageQueue and shutting off the LEDs
*/
void LedStateMachine::reset(void)
{
	m_State = eStateIdle;
	m_Preemptable = false;
	m_CurrentGroupId = 0xff;

	m_MessageQueue.reset();
	turnOffLeds();
}

/**
* Shut off the LEDs
*/
void LedStateMachine::turnOffLeds(void)
{
	m_SpiLeds.clear();
	m_SpiLeds.write();
}

/**
* Get the next message from the queue
*
* @return - a pointer to a Packet object, or NULL if repititions are exhausted
*/
Packet* LedStateMachine::nextMessage(void)
{
	if (++m_CurrentIndex == m_NumInGroup)
	{
		// Are we done
		if (0 == --m_Repetitions)
		{
			m_MessageQueue.consumerRelease();
			return NULL;
		}
		else
		{
			m_CurrentIndex = 0;
		}
	}
	return m_MessageQueue.retrieveNextMessage(m_CurrentMsg);
}

/**
* This updates the state machine
*
* @note - this is called by the main thread approx. every 10 ms.
*/
bool LedStateMachine::updateState(void)
{
	Packet *l_Msg;

	// check to see if the group need to be dismissed
	// This is usually done with the side button
	if (m_DismissGroup)
	{
		// only dismiss if the SM is active
		if (m_State != eStateIdle)
		{
			m_MessageQueue.consumerRelease();		// release 
			m_State = eStateIdle;
			m_CurrentGroupId = 0xff;
			m_Preemptable = false;
			turnOffLeds();
		}

		m_DismissGroup = false;
	}
	switch (m_State)
	{
		case eStateIdle:
			m_NumInGroup = 0;
			g_DisplayPower = 0;
			while (m_MessageQueue.get(&l_Msg))
			{
				if (0 == m_NumInGroup++)
				{
					// turn the LED display driver power on and then delay
					// for 10 ms
					m_State = eStateDelay;
					g_DisplayPower = 1;
					m_CountDown = 1;

					m_CurrentIndex = 0;
					m_CurrentMsg = l_Msg;
					m_Preemptable = m_CurrentMsg->getFlags() &  HeadsUpMessageProtocol::ePreemptable;
					m_CurrentGroupId = m_CurrentMsg->getGroupId();
					m_Repetitions = m_CurrentMsg->getRepetitions();
				}
				// last message - then leave
				if (l_Msg->getFlags() & HeadsUpMessageProtocol::eLastInGroupMask)
				{
					break;
				}
			}
			// return true if there are LED messages to process
			// return false if we are idle and there are no LED messages
			//
			return (m_NumInGroup != 0);
		case eStateDelay:
			if (0 == --m_CountDown)
			{
				m_State = eStateMessageBegin;
			}
			break;
		case eStateMessageBegin:
			m_EasingTime = m_CurrentMsg->getEasing();
			m_Duration = m_CurrentMsg->getDuration();
			if (m_EasingTime)
			{

				m_State = eStateEasing;
				m_CountDown = m_EasingTime;
				memcpy(m_EndLeds, m_CurrentMsg->getLeds(), sizeof(m_CurrentLeds));
				for (int i=0; i<RgbLed::m_NumberOfLeds; i++)
				{
					m_Easing[i].init(m_CurrentLeds[i], m_EndLeds[i], m_EasingTime);
				}
				for (int i=0; i<RgbLed::m_NumberOfLeds; i++)
				{
					m_Easing[i].calc(m_CurrentLeds[i]);
				}
			}
			else
			{
				m_State = eStateSteady;
				m_CountDown = m_Duration;
				memcpy(m_CurrentLeds, m_CurrentMsg->getLeds(), sizeof(m_CurrentLeds));
			}
			m_SpiLeds.setLeds(&m_CurrentLeds);
			m_SpiLeds.write();

			break;
		case eStateEasing:
			// are we done with Easing
			if (0 == --m_CountDown)
			{
				// reconcile that easing may have not ended precicely on the correct value
				// so just copy in the correct values 
				memcpy(m_CurrentLeds, m_CurrentMsg->getLeds(), sizeof(m_CurrentLeds));
				m_CountDown = m_Duration;
				if (m_CountDown)
				{
					m_State = eStateSteady;
					m_CountDown = m_Duration;
				}
				else
				{
					if (NULL != (m_CurrentMsg = nextMessage()))
					{
						m_State = eStateMessageBegin;
					}
					else
					{
						m_State = eStateIdle;
						m_Preemptable = false;
						m_CurrentGroupId = 0xff;
					}
				}
			}
			else
			{
				// Ease on down the road
				for (int i=0; i < RgbLed::m_NumberOfLeds; i++)
				{
					m_Easing[i].calc(m_CurrentLeds[i]);
				}
			}
			// write the new values
			m_SpiLeds.setLeds(&m_CurrentLeds);
			m_SpiLeds.write();
			break;
		case eStateSteady:
			if (0 == --m_CountDown)
			{
				// STEADY State is done so go to next MSG
				if (NULL != (m_CurrentMsg = nextMessage()))
				{
					m_State = eStateMessageBegin;
				}
				else
				{
					m_State = eStateIdle;
					m_Preemptable = false;
					m_CurrentGroupId = 0xff;
#if 0
					void (*__restart_vector)(void) = (void(*)())0xc0;
					__restart_vector();
#endif
				}
			}
			break;
		default:
			break;
	}
	return true;
}

