/**
* @file led_state_machine.h
* @brief defines the Easing and LedStateMachine objects
*
* Copyright (c) 2016 Heads Up Display, Inc
*/
#ifndef __LED_STATE_MACHINE__
#define __LED_STATE_MACHINE__

#include "mbed.h"
#include "hump.h"
#include "packet_queue.h"
#include "rgb_led.h"
#include "TLC59711.h"

/**
* The Easing class will control the rate and brightness of the LEDs
*/
class Easing
{
public:
	/**
	* Create the Easing object
	*/
	Easing() { clear(); }

	/**
	* The clear function will set the increment member variables to 0
	*/
	void clear(void) { m_RedInc = m_GreenInc = m_BlueInc = 0; }

	/**
	* The init function initializes the accumulator, increment and times member variables
	*
	* @param [in] a_StartLed - a RgbLed used to determine initial values for accumulator member variables
	* @param [in] a_EndLed - a RgbLed used to determine final values for increment member variables
	* @param [in] m_EasingTime - the total time the easing shall take to get from a_StartLed to a_EndLed
	*/
	void init(RgbLed& a_StartLed, RgbLed& a_EndLed, uint16_t m_EasingTime)
	{
		m_Times = 0;

		m_RedAccum = ((int32_t)a_StartLed.getRed()) << 15;
		m_GreenAccum = ((int32_t)a_StartLed.getGreen()) << 15;
		m_BlueAccum = ((int32_t)a_StartLed.getBlue()) << 15;

		m_RedInc = ((((int32_t)a_EndLed.getRed()) << 15) - m_RedAccum) / m_EasingTime;
		m_GreenInc = ((((int32_t)a_EndLed.getGreen()) << 15) - m_GreenAccum) / m_EasingTime;
		m_BlueInc = ((((int32_t)a_EndLed.getBlue()) << 15) - m_BlueAccum) / m_EasingTime;
	}

	/**
	* The calcFirst function is intended to smooth out the easing in a situation when the easing time 
	* does not fit into nicely even slices.	This will apply a fix to the first easing calculation.
	*
	* @param [out] a_Led - a RgbLed that will be updated based on the internal increment member variables
	*/
	void calcFirst(RgbLed& a_Led)
	{
		// Put the ticks in the right place, say easing is 2, so the ticks
		// are 1/4 and 3/4 so the first time we add half the easing value
		++m_Times;
		a_Led.setRed(((((int32_t) a_Led.getRed()) << 15) + m_RedInc/2) >> 15);
		a_Led.setGreen(((((int32_t) a_Led.getGreen()) << 15) + m_GreenInc/2) >> 15);
		a_Led.setBlue(((((int32_t) a_Led.getBlue()) << 15) + m_BlueInc/2) >> 15);
	}

	/**
	* The calc will update the internal increment and accumulator member variables
	*
	* @param [out] a_Led - a RgbLed that will be updated based on the internal increment and accumulator member variables
	*/
	void calc(RgbLed& a_Led)
	{
		++m_Times;

		m_RedAccum += m_RedInc;
		m_GreenAccum += m_GreenInc;
		m_BlueAccum += m_BlueInc;

		a_Led.setRed(m_RedAccum >> 15);
		a_Led.setGreen(m_GreenAccum >> 15);
		a_Led.setBlue(m_BlueAccum >> 15);
	}

protected:
	int m_Times;
	int32_t m_RedInc;
	int32_t m_GreenInc;
	int32_t m_BlueInc;
	int32_t m_RedAccum;
	int32_t m_GreenAccum;
	int32_t m_BlueAccum;
};

/**
* The LedStateMachine class will manage the LEDs
*/
class LedStateMachine
{
public:
	enum LedStateMachineStates
	{
		eStateIdle,
		eStateDelay,
		eStateMessageBegin,
		eStateEasing,
		eStateSteady
	};

	LedStateMachine(TLC59711& a_SpiLeds, PacketQueue& a_MsgQueue);
	void reset(void);
	void turnOffLeds(void);
	bool updateState(void);

	/**
 * Getter for the m_CurrentGroupId
 *
 * @return - a copy of m_CurrentGroupId
 */
	uint8_t getCurrentGroupId(void) { return m_CurrentGroupId; }
	/**
 * This will force m_DismissGroup true 
 */
	void dismissGroup(void)				{ m_DismissGroup = true;	}

	bool isActiveGroupPreemptable(void)	{ return m_Preemptable;		}

protected:
	Packet* nextMessage(void);

	TLC59711& m_SpiLeds;

	PacketQueue& m_MessageQueue;

	LedStateMachineStates m_State;
	uint16_t m_CountDown;
	uint16_t m_Duration;
	uint16_t m_EasingTime;
	uint16_t m_Repetitions;
	uint8_t m_NumInGroup;
	uint8_t m_CurrentIndex;

	Packet* m_CurrentMsg;
	Packet* m_CurrentCmd;
	uint8_t m_CurrentGroupId;
	bool m_DismissGroup;
	bool m_Preemptable;			// indicates if the active message is preemptable

	RgbLed m_EndLeds[RgbLed::m_NumberOfLeds];
	RgbLed m_CurrentLeds[RgbLed::m_NumberOfLeds];

	Easing m_Easing[RgbLed::m_NumberOfLeds];
};

#endif
