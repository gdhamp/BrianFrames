#include "LEDStateMachine.h"

#define LED0 (5)
#define LED1 (6)
#define LED2 (9)
#define LED3 (10)
#define LED4 (11)

LEDStep g_LED0Steps[] = 
{
//         Flags     Reps  Mag   Fade  Duration
  LEDStep(  0,         1,   0,    200,  0 ),
  LEDStep(  eLastInGroup,  0,   255,    100,  0 ),
  LEDStep(  0,         2,   0,    50,    0 ),
  LEDStep(  eLastInGroup,  0,   255,    50,  0 ),
   LEDStep(  0,         6,   0,    50,    0 ),
  LEDStep(  eLastInGroup,  0,   255,   50,  0 )
};

LEDStep g_LED1Steps[] = 
{
//        Flags     Reps  Mag   Fade  Duration
  LEDStep(  0,         1,   255,    50,    0 ),
  LEDStep(  eLastInGroup,  0,   0,    250,    0 ),
  LEDStep(  0,         2,   255,    50,    0 ),
  LEDStep(  eLastInGroup,  0,   0,    50,   0 ),
  LEDStep(  0,         15,   255,    20,    0 ),
  LEDStep(  eLastInGroup,  0,   0,    20,  0 )
};

LEDStep g_LED2Steps[] = 
{
//        Flags     Reps  Mag   Fade  Duration
  LEDStep(  0,         1,   0,    50,    0 ),
  LEDStep(  0,         0,   255,  50,     0),
  LEDStep(  eLastInGroup,  0,   0,    200,    0 ),
  LEDStep(  0,         2,   255,    50,    0 ),
  LEDStep(  eLastInGroup,  0,   0,    50,   0 ),
  LEDStep(  0,         30,   255,    10,    0 ),
  LEDStep(  eLastInGroup,  0,   0,    10,  0 )
};

LEDStep g_LED3Steps[] = 
{
//        Flags     Reps  Mag   Fade  Duration
  LEDStep(  0,         1,   0,    100,    0 ),
  LEDStep(  0,         0,   255,  50,    0),
  LEDStep(  eLastInGroup,  0,   0,    150,    0 ),
  LEDStep(  0,         2,   255,    50,    0 ),
  LEDStep(  eLastInGroup,  0,   0,    50,   0 ),
  LEDStep(  0,         10,   255,    30,    0 ),
  LEDStep(  eLastInGroup,  0,   0,    30,  0 )
};

LEDStep g_LED4Steps[] = 
{
//        Flags     Reps  Mag   Fade  Duration
  LEDStep(  0,         1,   0,    150,    0 ),
  LEDStep(  0,         0,   255,  50,    0),
  LEDStep(  eLastInGroup,  0,   0,    100,    0 ),
  LEDStep(  0,         2,   255,    50,    0 ),
  LEDStep(  eLastInGroup,  0,   0,    50,   0 ),
  LEDStep(  0,         12,   255,    25,    0 ),
  LEDStep(  eLastInGroup,  0,   0,    25,  0 )
};

LED g_LED0(LED0);
LEDQueue g_LED0Queue((LEDStep *)g_LED0Steps, sizeof(g_LED0Steps)/sizeof(LEDStep));
LedStateMachine g_LED0SM(g_LED0, g_LED0Queue);

LED g_LED1(LED1);
LEDQueue g_LED1Queue((LEDStep *)g_LED1Steps, sizeof(g_LED1Steps)/sizeof(LEDStep));
LedStateMachine g_LED1SM(g_LED1, g_LED1Queue);

LED g_LED2(LED2);
LEDQueue g_LED2Queue((LEDStep *)g_LED2Steps, sizeof(g_LED2Steps)/sizeof(LEDStep));
LedStateMachine g_LED2SM(g_LED2, g_LED2Queue);

LED g_LED3(LED3);
LEDQueue g_LED3Queue((LEDStep *)g_LED3Steps, sizeof(g_LED3Steps)/sizeof(LEDStep));
LedStateMachine g_LED3SM(g_LED3, g_LED3Queue);

LED g_LED4(LED4);
LEDQueue g_LED4Queue((LEDStep *)g_LED4Steps, sizeof(g_LED4Steps)/sizeof(LEDStep));
LedStateMachine g_LED4SM(g_LED4, g_LED4Queue);

void setup()
{
	Serial.begin(115200);
	Serial.println("begin");
	// initialize digital pin LED_BUILTIN as an output.
	pinMode(13, INPUT);
	pinMode(LED0, OUTPUT);
	pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);
	pinMode(LED3, OUTPUT);
	pinMode(LED4, OUTPUT);
}

// the loop function runs over and over again forever
void loop()
{
	g_LED0SM.updateState();
	g_LED1SM.updateState();
	g_LED2SM.updateState();
	g_LED3SM.updateState();
	g_LED4SM.updateState();
	delay(10);						// wait for a 1/10 second
}
