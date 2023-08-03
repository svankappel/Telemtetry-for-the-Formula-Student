/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file gps_controller.h
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief GPS Controller task receives data from the GPS module on the
 * 		  uart port and fill the GPS buffer with the received data
 * ---------------------------------------------------------------------
 * Telemetry system for the Valais Wallis Racing Team.
 * This file contains code for the onboard device of the telemetry
 * system. The system receives the data from the sensors on the CAN bus 
 * and the data from the GPS on a UART port. An SD Card contains a 
 * configuration file with all the system parameters. The measurements 
 * are sent via Wi-Fi to a computer on the base station. The measurements 
 * are also saved in a CSV file on the SD card. 
 *--------------------------------------------------------------------*/

#ifndef __GPS_CONTROLLER_H
#define __GPS_CONTROLLER_H

/*! GPS_Controller implements the GPS_Controller task
* @brief GPS_Controller read the UART form GPS and fill the sensorBuffer array 
*        
*/
void GPS_Controller(void);

/*! Task_GPS_Controller_Init implements the GPS_Controller task initialization
* @brief GPS_Controller thread initialization
*      
*/
void Task_GPS_Controller_Init( void );


#endif /*__GPS_CONTROLLER_H*/