#include "LEDStateMachine.h"

#define LED0 (3)
#define LED1 (5)
#define LED2 (6)
#define LED3 (9)
#define LED4 (10)
#define LED5 (11)

LEDStep LEDStep0[] = 
{
	LEDStep( 0,             10, 128,  20,  10 ),
	LEDStep( eLastInGroup,   0,  20, 100, 300 )
};

void setup()
{
	// initialize digital pin LED_BUILTIN as an output.
	pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop()
{
	digitalWrite(LED_BUILTIN, HIGH);	// turn the LED on (HIGH is the voltage level)
	delay(200);						// wait for a second
	digitalWrite(LED_BUILTIN, LOW);		// turn the LED off by making the voltage LOW
	delay(200);						// wait for a second
}
