#ifndef __UDP_CLIENT_H
#define __UDP_CLIENT_H

/*! UDP_Client implements the UDP Client task.
* @brief UDP_Client uses a BSD socket to send messages to a defined UDP
*		server. 
*		A sample message will be sent to the IP address defined on the 
*		configuration file as Peer address.
* 		This function is used on an independent thread.
*/
void UDP_Client();

/*! Task_UDP_Client_Init initializes the task UDP Client
*
* @brief UDP Client initialization
*/
void Task_UDP_Client_Init( void );
#endif /*__UDP_CLIENT_H*/