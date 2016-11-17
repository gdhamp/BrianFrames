#include "LEDStateMachine.h"

#define LED0 (5)
#define LED1 (6)
#define LED2 (6)
#define LED3 (9)
#define LED4 (10)
#define LED5 (11)

LED g_LED1(LED1);
LEDStep g_LED1Steps[] = 
{
// 				Flags			Reps	Mag		Fade	Duration
	LEDStep(	0,				 4,		255,  	80,  	100 ),
	LEDStep(	eLastInGroup,	 0,		77,		80,		100 ),
	LEDStep(	0,				 3,		88,  	80,  	100 ),
	LEDStep(	eLastInGroup,	 0,		22,		80,		100 )
};

LEDQueue g_LED1Queue((LEDStep *)g_LED1Steps, sizeof(g_LED1Steps)/sizeof(LEDStep));

LedStateMachine g_LED1SM(g_LED1, g_LED1Queue);

void setup()
{
	Serial.begin(115200);
	Serial.println("begin");
	// initialize digital pin LED_BUILTIN as an output.
	pinMode(13, INPUT);
	pinMode(LED1, OUTPUT);
}

// the loop function runs over and over again forever
void loop()
{
	g_LED1SM.updateState();
	delay(10);						// wait for a 1/10 second
}
