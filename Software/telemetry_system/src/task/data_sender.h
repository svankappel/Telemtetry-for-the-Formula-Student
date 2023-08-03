/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file data_sender.h
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief Data sender task. This task periodically sends the data from
 * 		  the sensor buffer and gps buffer to the udp client task
 * ---------------------------------------------------------------------
 * Telemetry system for the Valais Wallis Racing Team.
 * This file contains code for the onboard device of the telemetry
 * system. The system receives the data from the sensors on the CAN bus 
 * and the data from the GPS on a UART port. An SD Card contains a 
 * configuration file with all the system parameters. The measurements 
 * are sent via Wi-Fi to a computer on the base station. The measurements 
 * are also saved in a CSV file on the SD card. 
 *--------------------------------------------------------------------*/

#ifndef __DATA_SENDER_H
#define __DATA_SENDER_H

/*! Data_Sender implements the Data_Sender task
* @brief Data_Sender reads the data in the sensor buffer array and
*        creates the json string to send via Wi-Fi to the base 
*        station. This string is placed in the UDP_Client queue.
*/
void Data_Sender();


/*! Task_Data_Sender_Init initializes the task Data_Sender
*
* @brief Data Sender initialization
*/
void Task_Data_Sender_Init( void );

/*! data_Sender_timer_handler is called by the timer interrupt
* @brief data_Sender_timer_handler submit a new work that call Data_Sender task     
*/
void data_Sender_timer_handler();


#endif /*__DATA_SENDER_H*/