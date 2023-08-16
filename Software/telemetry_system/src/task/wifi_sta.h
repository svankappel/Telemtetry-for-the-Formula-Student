/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file wifi_sta.h
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief Wifi sta task connects the system to the configured WiFi
 * 		  Router and Redundancy WiFi Router. Connection status is
 * 		  saved in the deviceInformation.h file
 * ---------------------------------------------------------------------
 * Telemetry system for the Valais Wallis Racing Team.
 * This file contains code for the onboard device of the telemetry
 * system. The system receives the data from the sensors on the CAN bus 
 * and the data from the GPS on a UART port. An SD Card contains a 
 * configuration file with all the system parameters. The measurements 
 * are sent via Wi-Fi to a computer on the base station. The measurements 
 * are also saved in a CSV file on the SD card. 
 *--------------------------------------------------------------------*/

#ifndef WIFI_STA_H
#define WIFI_STA_H

/*! Wifi_Sta implements the task WiFi Stationing.
* 
* @brief Wifi_Stationing makes the complete connection process.
*/
void Wifi_Sta( void );

/*! Task_Wifi_Sta_Init initializes the task Wifi Stationing.
*
* @brief Wifi Stationing initialization
*/
void Task_Wifi_Sta_Init( void );

//functions prototypes

/*! @brief connection handler must be called after the wifi_connect request.
 *         It handles the connection callbacks.
*/
void connection_handler(void);

#endif /* WIFI_STA_H */