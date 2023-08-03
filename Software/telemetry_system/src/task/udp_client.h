/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file udp_client.h
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief Data logger task. This task reads periodically the sensor 
 *        buffer and puts the data in the CSV file
 * ---------------------------------------------------------------------
 * Telemetry system for the Valais Wallis Racing Team.
 * This file contains code for the onboard device of the telemetry
 * system. The system receives the data from the sensors on the CAN bus 
 * and the data from the GPS on a UART port. An SD Card contains a 
 * configuration file with all the system parameters. The measurements 
 * are sent via Wi-Fi to a computer on the base station. The measurements 
 * are also saved in a CSV file on the SD card. 
 *--------------------------------------------------------------------*/

#ifndef __UDP_CLIENT_H
#define __UDP_CLIENT_H

#include <zephyr/net/socket.h>

/*! UDP_Client implements the UDP Client task.
* @brief UDP_Client uses BSD sockets to send messages to one or multiple UDP
*		server(s). 
*		The thread send to the UDP servers all the data pushed on the udpQueue.
*/
void UDP_Client();

/*! Task_UDP_Client_Init initializes the task UDP Client
*
* @brief UDP Client initialization
*/
void Task_UDP_Client_Init( void );


//function prototype

/*! @brief UDP socket connect function
 *  @param udpClientSocket UDP Client socket variable
 *  @param serverAdress Socket Address struct
 */
void connectUDPSocket(int * udpClientSocket,struct sockaddr_in * serverAddress);



#endif /*__UDP_CLIENT_H*/