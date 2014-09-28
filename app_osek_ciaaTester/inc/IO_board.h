/*
 * IO_board.h
 *
 *  Created on: 05/09/2014
 *      Author: Administrador
 */

#ifndef IO_BOARD_H_
#define IO_BOARD_H_

/**
 * LED defines
 */
#define LED_RED			0x01
#define LED_YELLOW		0x02
#define LED_GREEN		0x04
#define LEDS_NO_LEDS	0x00

void IO_Action( uint8_t *pFrame);
void IO_Board_Init( void );

/**
 * @brief	Sets the state of a board LED to on or off
 * @param	LEDNumber	: LED number to set state for
 * @param	State		: true for on, false for off
 * @return	None
 */
void IO_Board_LED_OnOff(uint8_t LEDNumber, bool State);

/**
 * @brief	Returns the current state of a board LED
 * @param	LEDNumber	: LED number to set state for
 * @return	true if the LED is on, otherwise false
 */
bool IO_Board_LED_Test(uint8_t LEDNumber);

/**
 * @brief	Toggles the current state of a board LED
 * @param	LEDNumber	: LED number to change state for
 * @return	None
 */
void IO_Board_LED_Toggle(uint8_t LEDNumber);



#endif /* IO_BOARD_H_ */
