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
	LED(uint8_t a_Pin) : m_Pin(a_Pin) { clear(); }

	/**
	 * Clears the value of the LED
	 */
	void clear(void) { m_Magnitude = 0; }

	// TODO
	void write(void) {  }
	/**
	 * Set the value of the LED
	 * 
	 * @param [in] value - the value to set the LED to
	 */
	void setMagnitude(uint8_t value) {	m_Magnitude = value;
#if 0
		Serial.print(m_Magnitude);
		Serial.print('\t');
		Serial.println(millis());
#endif
										analogWrite(m_Pin, m_Magnitude);	}
	void setMagnitude(LED& a_LED) { setMagnitude(a_LED.getMagnitude());		}

	/**
	 * Getter for the m_Magnitude
	 *
	 * @return - a copy of m_Magnitude
	 */
	uint8_t getMagnitude(void) { return m_Magnitude; }

	static const uint8_t m_NumberOfLeds = 1;
protected:
	uint8_t m_Pin;
	uint8_t	m_Magnitude;
};

/**
* The LEDStep class is used to define a step in an LED sequence
*
*/
class LEDStep
{
public:
	LEDStep(uint8_t a_Flags, uint8_t a_Reps, uint8_t a_Magnitude, uint16_t a_Easing, uint16_t a_Duration);

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
	uint8_t getLEDMagnitude(void) { return m_LEDMagnitude; }

	/**
	* Getter for the m_Easing
	*
	* @return - a copy of m_Easing
	*/
	uint16_t getEasing(void) { return m_Easing; }

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
	uint8_t  m_LEDMagnitude;		// The Color for each LED
	uint16_t m_Easing;		// transition time in units of 1/100 sec
	uint16_t m_Duration;		// duration of this setting in units of 1/100 sec
};



/**
* The PacketQueue class will manage the packets in a queue
*/
class LEDQueue
{
public:
	LEDQueue(LEDStep* a_Buffer, int a_Count);
	virtual ~LEDQueue(void);

	void reset(void);
	void SetEndIndex(void)		{ m_GroupEndIndex = m_CurIndex; }
	LEDStep* get(bool a_Start);
	LEDStep* retrieveNextMessage(void);



protected:
	int m_Count;					// number of items in the queue
	
	LEDStep* m_Head;				// pointer to the head of the queue
	int m_CurIndex;

	int m_GroupCurIndex;
	int m_GroupStartIndex;
	int m_GroupEndIndex;

	LEDStep* m_ConStartIndex;		// consumer start index
	LEDStep* m_ConEndIndex;			// consumer end index + 1
	LEDStep* m_ConRdIndex;			// consumer end index

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
	Easing() { clear();  }

	void setLED(LED* a_LED) { m_LED = a_LED;	}

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
	void init(uint8_t a_StartMag, uint8_t a_EndMag, uint16_t m_EasingTime)
	{
		m_Times = 0;

		m_Accum = ((int32_t)a_StartMag) << 15;

		m_Inc = ((((int32_t)a_EndMag) << 15) - m_Accum) / m_EasingTime;
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
		m_LED->setMagnitude(((((int32_t) a_Led.getMagnitude()) << 15) + m_Inc/2) >> 15);
	}

	/**
	* The calc will update the internal increment and accumulator member variables
	*
	* @param [out] a_Led - a LED that will be updated based on the internal increment and accumulator member variables
	*/
	void calc(void)
	{
		++m_Times;

		m_Accum += m_Inc;

		m_LED->setMagnitude(m_Accum >> 15);
	}

protected:
	int m_Times;
	LED* m_LED;
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

	LedStateMachine(LED& a_LED, LEDQueue& a_Steps);
	void reset(void);
	void turnOffLed(void);
	bool updateState(void);


protected:
	LEDStep* nextMessage(void);

	LED& m_LED;

	LEDQueue& m_LEDQueue;

	LedStateMachineStates m_State;
	uint16_t m_CountDown;
	uint16_t m_Duration;
	uint16_t m_EasingTime;
	uint16_t m_Repetitions;
	uint8_t m_NumInGroup;
	uint8_t m_CurrentIndex;

	LEDStep* m_CurrentMsg;
	LED* m_CurrentCmd;

	uint8_t m_EndLed;
	uint8_t m_CurrentLed;

	Easing m_Easing;
};

#pragma pack(pop)
#endif
