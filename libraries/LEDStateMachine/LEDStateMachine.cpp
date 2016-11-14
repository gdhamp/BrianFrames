
#include "Arduino.h"
#include "LEDStateMachine.h"



LEDStep::LEDStep(uint8_t a_Flags, uint8_t a_Reps, uint8_t a_Magnitude, uint16_t a_FadeTime, uint16_t a_Duration)
	: m_Flags(a_Flags), m_Repetitions(a_Reps), m_LEDMagnitude(a_Magnitude), m_FadeTime(a_FadeTime), m_Duration(a_Duration)
{
}






/**
* Create the LedStateMachine object, and reset the m_MessageQueue
*
* @param [in] a_SpiLeds - a TLC59711 shared between this object and others
* @param [in] a_MessageQueue - a PacketQueue shared between this object and others
*/
LedStateMachine::LedStateMachine(LED& a_LED, LEDStep& a_Steps) : m_LED(a_LED), m_Steps(a_Steps), m_DismissGroup(false)
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

	// TODO
	turnOffLed();
}

/**
* Shut off the LEDs
*/
void LedStateMachine::turnOffLed(void)
{
	m_LED.clear();
	m_LED.write();
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
			m_Preemptable = false;
			turnOffLeds();
		}

		m_DismissGroup = false;
	}
	switch (m_State)
	{
		case eStateIdle:
			m_NumInGroup = 0;
			while (m_MessageQueue.get(&l_Msg))
			{
				if (0 == m_NumInGroup++)
				{
					// turn the LED display driver power on and then delay
					// for 10 ms
					m_State = eStateDelay;
					m_CountDown = 1;

					m_CurrentIndex = 0;
					m_CurrentMsg = l_Msg;
					m_Preemptable = m_CurrentMsg->getFlags() &  HeadsUpMessageProtocol::ePreemptable;
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
			m_LED.setLeds(&m_CurrentLeds);
			m_LED.write();

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
			m_LED.setLeds(&m_CurrentLeds);
			m_LED.write();
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
				}
			}
			break;
		default:
			break;
	}
	return true;
}

