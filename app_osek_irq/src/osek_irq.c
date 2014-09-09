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

void setupHardware(void)
{
	/* Pulsador en P0.18, lo usamos por interrupcion */
	Chip_GPIOINT_Init(LPC_GPIOINT);

	Chip_GPIOINT_SetIntFalling(LPC_GPIOINT, GPIOINT_PORT0, 1<<18);

	NVIC_EnableIRQ(EINT3_IRQn);
}

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

    setupHardware();

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
	/* Activate TaskBlink */
	printf("InitTask: Activate TaskBlink.\n");
	ActivateTask(TaskBlink);

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

void EINT3_IRQHandler(void)
{
	if(Chip_GPIOINT_GetIntFalling(LPC_GPIOINT, GPIOINT_PORT0) & (1<<18))
	{
		Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, GPIOINT_PORT0, 1<<18);
		SetEvent(TaskBlink, evBlink);
	}
}

