
#include "Arduino.h"
#include "LEDStateMachine.h"



LEDStep::LEDStep(uint8_t a_Flags, uint8_t a_Reps, uint8_t a_Magnitude, uint16_t a_Easing, uint16_t a_Duration)
	: m_Flags(a_Flags), m_Repetitions(a_Reps), m_LEDMagnitude(a_Magnitude), m_Easing(a_Easing), m_Duration(a_Duration)
{
}



/**
* Create a LEDQueue object with an array of packets
*
* @param a_Buffer - an array of Packets
* @param a_Count - number of items packet array
*/
LEDQueue::LEDQueue(LEDStep* a_Buffer, int a_Count)
{
	// Set up the fixed stuff
	
	// store pointers to keep track of the queue
	m_Head = a_Buffer;
	// set the tail/clear the indices/set the size and length
	// to be clear, m_Tail points to the "item" that is 1 past
	// the last item
	m_Count = a_Count;

	//Reset the dynamic stuff
	reset();
}

/**
* Destructor
*/
LEDQueue::~LEDQueue()
{
}

/**
* Reset the queue
*/
void LEDQueue::reset(void)
{
	m_CurIndex = 0;
	m_GroupCurIndex = 0;
	m_GroupStartIndex = 0;
	m_GroupEndIndex = 0;
}


/**
* Get an item from the queue in an IRQ handler
*
* @return true if item fetched, false if queue is empty
*/
LEDStep* LEDQueue::get(bool a_Start)
{
	LEDStep* l_RetVal;

	if (a_Start)
	{
		m_GroupStartIndex = m_CurIndex;
		m_GroupCurIndex = m_CurIndex;
	}

	l_RetVal = &m_Head[m_CurIndex];

	if (++m_CurIndex >= m_Count)
		m_CurIndex = 0;

	return l_RetVal;
}

/**
* Just retrieve the next packet without modifying
* queue data. Returns a pointer to the next packet
*
* @note This is bit complicated.
*  m_ProdRdIndex is the first packet of the group
*  m_ConRdIndex is the packet past the last packet of the group
*
* @param packet pointer to the current packet
* @return pointer to the next packet (possibly wrapped around)
*/
LEDStep* LEDQueue::retrieveNextMessage(void)
{
	if (++m_GroupCurIndex >= m_Count)
		m_GroupCurIndex = 0;

	if (m_GroupCurIndex == m_GroupEndIndex)
		m_GroupCurIndex = m_GroupStartIndex;

	return &m_Head[m_GroupCurIndex];
}





/**
* Create the LedStateMachine object, and reset the m_LEDQueue
*
* @param [in] a_SpiLeds - a TLC59711 shared between this object and others
* @param [in] a_LEDQueue - a LEDQueue shared between this object and others
*/
LedStateMachine::LedStateMachine(LED& a_LED, LEDQueue& a_Steps) : m_LED(a_LED), m_LEDQueue(a_Steps)
{
	// Note - RgbLeds are clear by their constructor
	m_Easing.setLED(&m_LED);
	reset();
}

/**
* Reset all member variables, including resetting the m_LEDQueue and shutting off the LEDs
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
}

/**
* Get the next message from the queue
*
* @return - a pointer to a Packet object, or NULL if repititions are exhausted
*/
LEDStep* LedStateMachine::nextMessage(void)
{
	if (++m_CurrentIndex == m_NumInGroup)
	{
		// Are we done
		if (0 == --m_Repetitions)
		{
			return NULL;
		}
		else
		{
			m_CurrentIndex = 0;
		}
	}
	return m_LEDQueue.retrieveNextMessage();
}

/**
* This updates the state machine
*
* @note - this is called by the main thread approx. every 10 ms.
*/
bool LedStateMachine::updateState(void)
{
	LEDStep *l_Msg;

	switch (m_State)
	{
		case eStateIdle:
			m_NumInGroup = 0;
			while (1)
			{
				l_Msg = m_LEDQueue.get(m_NumInGroup == 0);
				if (0 == m_NumInGroup++)
				{
					// turn the LED display driver power on and then delay
					// for 10 ms
					m_State = eStateDelay;
					m_CountDown = 1;

					m_CurrentIndex = 0;
					m_CurrentMsg = l_Msg;
					m_Repetitions = m_CurrentMsg->getRepetitions();
				}
				// last message - then leave
				if (l_Msg->getFlags() & LEDMasks::eLastInGroup)
				{
					m_LEDQueue.SetEndIndex();
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
				m_EndLed = m_CurrentMsg->getLEDMagnitude();

				m_Easing.init(m_CurrentLed, m_EndLed, m_EasingTime);
				m_Easing.calc();
			}
			else
			{
				m_State = eStateSteady;
				m_CountDown = m_Duration;
				m_CurrentLed = m_CurrentMsg->getLEDMagnitude();
			}
			m_LED.setMagnitude(m_CurrentLed);

			break;
		case eStateEasing:
			// are we done with Easing
			if (0 == --m_CountDown)
			{
				// reconcile that easing may have not ended precicely on the correct value
				// so just copy in the correct values 
				m_CurrentLed = m_CurrentMsg->getLEDMagnitude();
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
					}
				}
			}
			else
			{
				// Ease on down the road
				m_Easing.calc();
			}
			// write the new values
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
				}
			}
			break;
		default:
			break;
	}
	return true;
}

