/*
 * IO_board.c
 *
 *  Created on: 05/09/2014
 *      Author: Administrador
 */


/* C includes */
#include <stdio.h>
#include <stdint.h>

/* lpc17xx includes */
#include "chip.h"
#include "chip_lpc175x_6x.h"

/* local includes */
#include "IO_board.h"

#define PORT0			0
#define PORT2			2
#define MASK_OPTO_IN_1	(1<<2)
#define MASK_OPTO_IN_2	(1<<3)
#define MASK_OPTO_IN_3	(1<<21)
#define MASK_OPTO_IN_4	(1<<22)
#define MASK_OPTO_IN_5	(1<<24)
#define MASK_OPTO_IN_6	(1<<25)
#define MASK_OPTO_IN_7	(1<<12)
#define MASK_OPTO_IN_8	(1<<13)
#define MASK_OUT_1 		(1<<3)
#define MASK_OUT_2 		(1<<4)
#define MASK_OUT_3 		(1<<5)
#define MASK_OUT_4 		(1<<6)
#define MASK_OUT_5 		(1<<7)
#define MASK_OUT_6 		(1<<8)
#define MASK_OUT_7 		(1<<10)
#define MASK_OUT_8 		(1<<11)

const uint32_t MASK_INPUTS[] = {MASK_OPTO_IN_1, MASK_OPTO_IN_2, MASK_OPTO_IN_3, MASK_OPTO_IN_4, MASK_OPTO_IN_5, MASK_OPTO_IN_6, MASK_OPTO_IN_7, MASK_OPTO_IN_8};
const uint32_t MASK_OUTPUTS[] = {MASK_OUT_1, MASK_OUT_2, MASK_OUT_3, MASK_OUT_4, MASK_OUT_5, MASK_OUT_6, MASK_OUT_7, MASK_OUT_8};

#define ST_WAITING_STX	0
#define ST_WAITING_ETX	1

#define STX 	0x02
#define ETX 	0x03
#define ACK 	0x06
#define NACK 	0x15

/** \fn void IO_Board_Init( void )
 *  \brief
 *  \param void
 *  \return void
 *
*/
void IO_Board_Init( void )
{
	/*! Initializes UART pins*/
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 16, (IOCON_FUNC1 | IOCON_MODE_INACT)); /* RXD1 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 15, (IOCON_FUNC1 | IOCON_MODE_INACT)); /* TXD1 */
	/* Initializes the pUART peripheral */
	Chip_UART_Init(LPC_UART1);
	Chip_UART_SetBaud(LPC_UART1, 115200);
	/* Enable transmission on UART TxD pin */
	Chip_UART_TXEnable(LPC_UART1);
	/*
	 * Initialize PORT 0 as inputs (P0.2, P0.3, P0.21, P0.22, P0.24, P0.25)
	 */
	Chip_GPIO_SetPortDIR(LPC_GPIO, PORT0, (1<<2|1<<3|1<<21|1<<22|1<<24|1<<25), 0 /*0: input*/);
	/*
	 * Initialize PORT 2 as inputs (P2.12, P2.13)
	 */
	Chip_GPIO_SetPortDIR(LPC_GPIO, PORT2, (1<<12|1<<13), 0 /*0: input*/);
	/*
	 * Initialize PORT 2 as outputs (P2.0 red led, P2.1 green led, UDN2981 outputs: P2.3, P2.4, P2.5, P2.6, P2.7, P2.8, P2.10, P2.11)
	 * All outputs low
	 */
	Chip_GPIO_SetPortDIR(LPC_GPIO, PORT2, (1<<0|1<<1|1<<3|1<<4|1<<5|1<<6|1<<7|1<<8|1<<10|1<<11), 1 /*1: output */);
	Chip_GPIO_ClearValue(LPC_GPIO, PORT2, (1<<0|1<<1|1<<3|1<<4|1<<5|1<<6|1<<7|1<<8|1<<10|1<<11));
	/*
	 * Initialize PORT 0 as output (P0.26 yellow led)
	 * All outputs low
	 */
	Chip_GPIO_SetPortDIR(LPC_GPIO, PORT0, (1<<26), 1 /*1: output */);
	Chip_GPIO_ClearValue(LPC_GPIO, PORT0, (1<<26));
}

/* Sets the state of a board LED to on or off */
void IO_Board_LED_OnOff(uint8_t LEDNumber, bool action)
{
	switch(LEDNumber)
	{
	case LED_RED:
		Chip_GPIO_WritePortBit(LPC_GPIO, PORT2, 0 , action);
		break;
	case LED_YELLOW:
		Chip_GPIO_WritePortBit(LPC_GPIO, PORT0, 26 , action);
		break;
	case LED_GREEN:
		Chip_GPIO_WritePortBit(LPC_GPIO, PORT2, 1 , action);
		break;
	}
}

/* Returns the current state of a board LED */
bool IO_Board_LED_Test(uint8_t LEDNumber)
{
	bool state = false;

	switch(LEDNumber)
	{
	case LED_RED:
		state = Chip_GPIO_ReadPortBit(LPC_GPIO, PORT2, 0);
		break;
	case LED_YELLOW:
		state = Chip_GPIO_ReadPortBit(LPC_GPIO, PORT0, 26);
		break;
	case LED_GREEN:
		state = Chip_GPIO_ReadPortBit(LPC_GPIO, PORT2, 1);
		break;
	}

	return state;
}

void IO_Board_LED_Toggle(uint8_t LEDNumber)
{
	IO_Board_LED_OnOff(LEDNumber, !IO_Board_LED_Test(LEDNumber));
}

void IO_Action( uint8_t *pFrame)
{
	uint8_t responseBuffer[10];
	uint32_t auxData;

	switch( *pFrame)
	{
		case 0x41: /* command: 'A' Read digital inputs*/
			pFrame++;
			if( *pFrame >= '1' && *pFrame <= '8'){
				responseBuffer[0] = ACK;

				if( *pFrame >= '1' && *pFrame <= '6'){
					auxData = Chip_GPIO_ReadValue(LPC_GPIO, PORT0);	/*inputs '1' to '6' from port 0*/
				}
				else{
					auxData = Chip_GPIO_ReadValue(LPC_GPIO, PORT2);	/*inputs '7' and '8' from port 2*/
				}

				if( auxData & MASK_INPUTS[ (*pFrame - '0' -1) ] ){
					responseBuffer[1] = '1';
				}
				else{
					responseBuffer[1] = '0';
				}
				responseBuffer[2] = '\0';
			}
			else{
				responseBuffer[0] = NACK;
				responseBuffer[1] = '\0';
			}
			break;

		case 0x42: /* command: 'B' Write digital outputs*/
			pFrame++;
			if( *pFrame >= '1' && *pFrame <= '8'){
				auxData = *pFrame-'0'-1;
				pFrame++;
				if( *pFrame - '0'){
					Chip_GPIO_SetValue(LPC_GPIO, PORT2, MASK_OUTPUTS[auxData]);
				}
				else{
					Chip_GPIO_ClearValue(LPC_GPIO, PORT2, MASK_OUTPUTS[auxData]);
				}

				responseBuffer[0] = ACK;
				responseBuffer[1] = '\0';
			}
			else{
				responseBuffer[0] = NACK;
				responseBuffer[1] = '\0';
			}
			break;
		default:
			responseBuffer[0] = NACK;
			responseBuffer[1] = '\0';
	}
	Chip_UART_SendBlocking( LPC_UART1, (uint8_t *)responseBuffer, strlen((char *)responseBuffer) );
}

