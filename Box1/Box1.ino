#include "LEDStateMachine.h"

#define LED0 (5)
#define LED1 (6)

LEDStep g_LED0Steps[] = 
{
// 				Flags			Reps	Mag		Fade	Duration
	LEDStep(	0,				 1,		255,  	200,  	200 ),
	LEDStep(	eLastInGroup,	 0,		0,		200,	200 ),
};

LEDStep g_LED1Steps[] = 
{
// 				Flags			Reps	Mag		Fade	Duration
	LEDStep(	0,				 1,		0,  	200,  	200 ),
	LEDStep(	eLastInGroup,	 0,		255,		200,		200 ),
};

LED g_LED0(LED0);
LEDQueue g_LED0Queue((LEDStep *)g_LED0Steps, sizeof(g_LED0Steps)/sizeof(LEDStep));
LedStateMachine g_LED0SM(g_LED0, g_LED0Queue);

LED g_LED1(LED1);
LEDQueue g_LED1Queue((LEDStep *)g_LED1Steps, sizeof(g_LED1Steps)/sizeof(LEDStep));
LedStateMachine g_LED1SM(g_LED1, g_LED1Queue);

void setup()
{
	Serial.begin(115200);
	Serial.println("begin");
	// initialize digital pin LED_BUILTIN as an output.
	pinMode(13, INPUT);
	pinMode(LED0, OUTPUT);
	pinMode(LED1, OUTPUT);
}

// the loop function runs over and over again forever
void loop()
{
	g_LED0SM.updateState();
	g_LED1SM.updateState();
	delay(10);						// wait for a 1/10 second
}
