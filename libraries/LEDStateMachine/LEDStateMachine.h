/**
* @file LEDStateMachine
* @brief defines the clases for the LED State Machine
*
*/
#ifndef __LEDSTATEMACHINE_H__
#define __LEDSTATEMACHINE_H__


#pragma pack(push, 1)

enum LEDMasks
{
	eLastInGroup = 0x80,	// For messages, this indicates whether this is the last message
};


class LED
{
public:
	/**
	 * Create the LED object, and clear it
	 */
	LED() { clear(); }

	/**
	 * Create the LED object, with initial values for the magnitude
	 *
	 * @param [in] a_Magnitude - the value to initialize the LED to
	 */
	LED(uint8_t a_Magnitude)
	{
		m_Magnitude = a_Magnitude;
	}

	/**
	 * Clears the value of the LED
	 */
	void clear(void) { m_Magnitude = 0; }
	/**
	 * Set the value of the LED
	 * 
	 * @param [in] value - the value to set the LED to
	 */
	void setMagnitude(uint8_t value) { m_Magnitude = value; }

	/**
	 * Getter for the m_Magnitude
	 *
	 * @return - a copy of m_Magnitude
	 */
	uint8_t getMagnitude(void) { return m_Magnitude; }

	static const uint8_t m_NumberOfLeds = 2;
protected:
	uint8_t	m_Magnitude;
};

/**
* The LEDStep class is used to define a step in an LED sequence
*
*/
class LEDStep
{
public:
	LEDStep(uint8_t a_Flags, uint8_t a_Reps, uint8_t a_Magnitude, uint16_t a_FadeTime, uint16_t a_Duration);

	/**
	* Getter for the m_Flags
	*
	* @return - a copy of m_Flags
	*/
	uint8_t getFlags(void) { return m_Flags; }


	/**
	* Getter for the m_Repetitions
	*
	* @return - a copy of m_Repetitions
	*/
	uint8_t getRepetitions(void) { return m_Repetitions; }

	/**
	* Getter for the m_Leds
	*
	* @return - a copy of m_Leds
	*/
	LED& getLedMagniture(void) { return m_LEDMagnitude; }

	/**
	* Getter for the m_FadeTime
	*
	* @return - a copy of m_FadeTime
	*/
	uint16_t getFadeTime(void) { return m_FadeTime; }

	/**
	* Getter for the m_Duration
	*
	* @return - a copy of m_Duration
	*/
	uint16_t getDuration(void) { return m_Duration; }

protected:
	uint8_t m_Flags;			// bit definitions defined above
	uint8_t m_Repetitions;		// for message - number of repetitions for the group

	// All of this is for messages
	LED  m_LEDMagnitude;		// The Color for each LED
	uint16_t m_FadeTime;		// transition time in units of 1/100 sec
	uint16_t m_Duration;		// duration of this setting in units of 1/100 sec
};



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
	void clear(void) { m_Inc = 0; }

	/**
	* The init function initializes the accumulator, increment and times member variables
	*
	* @param [in] a_StartLed - a LED used to determine initial values for accumulator member variables
	* @param [in] a_EndLed - a LED used to determine final values for increment member variables
	* @param [in] m_EasingTime - the total time the easing shall take to get from a_StartLed to a_EndLed
	*/
	void init(LED& a_StartLed, LED& a_EndLed, uint16_t m_EasingTime)
	{
		m_Times = 0;

		m_Accum = ((int32_t)a_StartLed.getMagnitude()) << 15;

		m_Inc = ((((int32_t)a_EndLed.getMagnitude()) << 15) - m_Accum) / m_EasingTime;
	}

	/**
	* The calcFirst function is intended to smooth out the easing in a situation when the easing time 
	* does not fit into nicely even slices.	This will apply a fix to the first easing calculation.
	*
	* @param [out] a_Led - a LED that will be updated based on the internal increment member variables
	*/
	void calcFirst(LED& a_Led)
	{
		// Put the ticks in the right place, say easing is 2, so the ticks
		// are 1/4 and 3/4 so the first time we add half the easing value
		++m_Times;
		a_Led.setMagnitude(((((int32_t) a_Led.getMagnitude()) << 15) + m_Inc/2) >> 15);
	}

	/**
	* The calc will update the internal increment and accumulator member variables
	*
	* @param [out] a_Led - a LED that will be updated based on the internal increment and accumulator member variables
	*/
	void calc(LED& a_Led)
	{
		++m_Times;

		m_Accum += m_Inc;

		a_Led.setMagnitude(m_Accum >> 15);
	}

protected:
	int m_Times;
	int32_t m_Inc;
	int32_t m_Accum;
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

	LedStateMachine(LED& a_LED, LEDStep& a_Steps);
	void reset(void);
	void turnOffLed(void);
	bool updateState(void);

	void dismissGroup(void)				{ m_DismissGroup = true;	}

protected:
	LEDStep* nextMessage(void);

	LED& m_LED;

	LEDStep& m_Steps;

	LedStateMachineStates m_State;
	uint16_t m_CountDown;
	uint16_t m_Duration;
	uint16_t m_EasingTime;
	uint16_t m_Repetitions;
	uint8_t m_NumInGroup;
	uint8_t m_CurrentIndex;

	LED* m_CurrentMsg;
	LED* m_CurrentCmd;
	bool m_DismissGroup;

	LED m_EndLed[LED::m_NumberOfLeds];
	LED m_CurrentLed[LED::m_NumberOfLeds];

	Easing m_Easing[LED::m_NumberOfLeds];
};

#pragma pack(pop)
#endif
