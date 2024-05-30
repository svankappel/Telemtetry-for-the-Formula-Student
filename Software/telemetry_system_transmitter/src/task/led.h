/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file led.h
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief Led task controlls leds. The led1 blinks if the wifi is 
 *        connected and led2 blinks if logs are currently recording. 
 *        if an error occured during the configuration file read, the 
 *        led2 stays on.
 * ---------------------------------------------------------------------
 * Telemetry system for the Valais Wallis Racing Team.
 * This file contains code for the onboard device of the telemetry
 * system. The system receives the data from the sensors on the CAN bus 
 * and the data from the GPS on a UART port. An SD Card contains a 
 * configuration file with all the system parameters. The measurements 
 * are sent via Wi-Fi to a computer on the base station. The measurements 
 * are also saved in a CSV file on the SD card. 
 *--------------------------------------------------------------------*/
#ifndef __LED_H
#define __LED_H

/*! Led implements the Led task
* @brief Led blink led1 when wifi is connected,
*        blink led2 when data logging is active
*		 and set led2 on when config file read is failed
*/
void Led(void);

/*! Task_Led_Init implements the Led task initialization
* @brief Led thread initialization
*/
void Task_Led_Init( void );

#endif /*__LED_H*/