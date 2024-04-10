/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file deviceInformation.h
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief deviceInformation file contains a struct with the wifi
 * 		  connection status
 * ---------------------------------------------------------------------
 * Telemetry system for the Valais Wallis Racing Team.
 * This file contains code for the onboard device of the telemetry
 * system. The system receives the data from the sensors on the CAN bus 
 * and the data from the GPS on a UART port. An SD Card contains a 
 * configuration file with all the system parameters. The measurements 
 * are sent via Wi-Fi to a computer on the base station. The measurements 
 * are also saved in a CSV file on the SD card. 
 *--------------------------------------------------------------------*/

#ifndef __DEVICEINFORMATION_H
#define __DEVICEINFORMATION_H

#include <stdio.h>

/*! @struct sContext
 * @brief Data structure with the Wifi stationing context information
 * @typedef tContext
 * @brief Data type with the Wifi stationing context information
*/
typedef struct sContext{
	//! Pointer to the sell
	const struct shell *pShell;
	union {
		struct {
			//! Connected 
			uint8_t connected					    : 					      1;
			uint8_t connect_result				    : 					      1;
			uint8_t disconnect_requested		    : 					      1;
			uint8_t ip_assigned						:						  1;
			uint8_t _unused						    : 					      4;
		};
		uint8_t all;
	};
}tContext;

//! Contains the context information of the Wifi stationing
extern tContext context;


#endif /*__DEVICEINFORMATION_H*/