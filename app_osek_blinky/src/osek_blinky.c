/*
===============================================================================
 Name        : osek_example_app.c
 Author      : Pablo Ridolfi
 Version     :
 Copyright   : (c)2014, Pablo Ridolfi, DPLab@UTN-FRBA
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <stdio.h>
#include <cr_section_macros.h>

#include "os.h"               /* <= operating system header */

/*
 * See the OIL configuration file in
 * etc/config.oil
 */

/* Comment this line in order to use LPC-Link based printf()
 * If uncommented, all printf() calls will be disabled
 */
//#define printf(...)

int main(void)
{
#if defined (__USE_LPCOPEN)
#if !defined(NO_BOARD_LIB)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "On"
    Board_LED_Set(0, false);
#endif
#endif

    printf("Starting OSEK-OS in AppMode1\n");
    StartOS(AppMode1);

    /* we shouldn't return here */
    while(1);

    return 0 ;
}


void ErrorHook(void)
{
	/* kernel panic :( */
	ShutdownOS(0);
}

TASK(TaskInit)
{
	/* Set 500 tick alarm for TaskPeriodic */
	printf("InitTask: SetRelAlarm for TaskPeriodic.\n");
	SetRelAlarm(ActivateTaskPeriodic, 0, 500);

	/* Set 1000 tick callback alarm */
	printf("InitTask: SetRelAlarm for AppCallback.\n");
	SetRelAlarm(AppCallbackAlarm, 100, 1000);

	/* Activate TaskBlink */
	printf("InitTask: Activate TaskBlink.\n");
	ActivateTask(TaskBlink);

	/* Activate TaskBackground */
	ActivateTask(TaskBackground);

	/* end InitTask */
	printf("InitTask: TerminateTask().\n");
	TerminateTask();
}

/*
 * This task waits for an event to be set in order
 * to continue execution.
 */
TASK(TaskBlink)
{
	printf("TaskBlink: Init.\n");
	while(1)
	{
		printf("TaskBlink: Waiting for event...\n");
		WaitEvent(evBlink);
		ClearEvent(evBlink);
		printf("TaskBlink: LED Toggle.\n");
		Board_LED_Toggle(0);
	}
	TerminateTask();
}

/*
 * This is a periodic task.
 */
TASK(TaskPeriodic)
{
	printf("TaskPeriodic: Event set.\n");
	SetEvent(TaskBlink, evBlink);

	/* end TaskPeriodic */
	TerminateTask();
}

/*
 * Just a background task with an infinite loop,
 * it has to be defined with the minimum priority!!!
 */
TASK(TaskBackground)
{
	volatile int i = 0;
	printf("TaskBackground: Running!\n");
	while(1)
	{
		i++;
		if(i == 0xFFFFF)
		{
			printf("TaskBackground still running...\n");
			i = 0;
		}
	}
}

/*
 * Alarm Callback example.
 */
ALARMCALLBACK(AppCallback)
{
	printf("AppCallback.\n");
}
