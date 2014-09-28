/* Copyright 2014, Gustavo Alessandrini
 *
 * This file is part of CIAA Firmware.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/** \brief IO Tester main definiton source file
 **
 **/

/** \addtogroup CIAA_Firmware CIAA IO Tester
 ** @{ */
/** \addtogroup Testing CIAA Firmware
 ** @{ */
/** \addtogroup Testing IO Tester source file
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 * GuAl         Gustavo Alessandrini
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20140909 v0.0.1   GuAl first functional version
 */

/*==================[inclusions]=============================================*/
#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <cr_section_macros.h>

#include "os.h"               /* <= operating system header */

/* local includes */
#include "IO_board.h"
#include "uart_17xx_40xx.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

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
    /* Read clock settings and update SystemCoreClock variable*/
    SystemCoreClockUpdate();
    /* Set up and initialize all required blocks and functions related to the board hardware */
    Board_Init();		/* Set up LPC1769 stick ga: P0.26 as GPIO*/
    IO_Board_Init();	/* Set up daughter IO board */

    /* Set the STICK LED to the state of "On" */
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

/** \brief Task Init
 * This task is started automatically in the application mode 1.
 */
TASK(TaskInit)
{
	/* Activate TaskBackground */
	ActivateTask(TaskBackground);

	/* end InitTask */
	printf("InitTask: TerminateTask().\n");
	TerminateTask();
}

/** \brief Task Blinking
 * This task performs a blinking keep alive.
 */
TASK(TaskBlinking)
{
	IO_Board_LED_Toggle( LED_RED );
	TerminateTask();
}

/** \brief Task Background
 * This task waits for inputs commands from uart1, performs the action, and writes
 * the response to uart1
 *
 * Just a background task with an infinite loop,
 * it has to be defined with the minimum priority!!!
 */
#define ST_WAITING_STX	0
#define ST_WAITING_ETX	1

#define STX 	0x02
#define ETX 	0x03

TASK(TaskBackground)
{
	uint8_t serialIn;
	uint8_t cmdBufIndex = 0;
	uint8_t estado = ST_WAITING_STX;
	uint8_t commandBuffer[10];
	int32_t readBytes;

	printf("TaskBackground: Running!\n");
	Chip_UART_SendBlocking( LPC_UART1, (uint8_t *)"ready...\n\r", strlen("ready...\n\r") );

	while(1)
	{
		if( 0 != (readBytes = Chip_UART_Read(LPC_UART1, &serialIn, 1))){
			switch(estado)
			{
			case ST_WAITING_STX:
				if(serialIn == STX ){
					cmdBufIndex = 0;
					estado = ST_WAITING_ETX;
				}
				break;
			case ST_WAITING_ETX:
				if(serialIn != ETX ){
					commandBuffer[ cmdBufIndex ] = serialIn;
					if( cmdBufIndex++ >= 10 ){
						estado = ST_WAITING_STX;
					}
				}
				else{
					IO_Action(commandBuffer);
					IO_Board_LED_OnOff(LED_GREEN, 1/*on*/);

					/* Set 200 tick callback alarm */
					printf("InitTask: SetRelAlarm for AppCallback. One shot\n");
					SetRelAlarm(AppCallbackAlarm, 200, 0/*one shot*/);

					estado = ST_WAITING_STX;
				}
				break;
			}
		}
	}
}

/** \brief Alarm CallBack
 * This function is callback from alarm, to turn off green led (A new command was completed)
 */
ALARMCALLBACK(AppCallback)
{
	printf("AppCallBack: LED off.\n");
	IO_Board_LED_OnOff(LED_GREEN, 0/*off*/);
}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/


