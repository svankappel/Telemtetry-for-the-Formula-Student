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

//functions prototypes
void connectUDPSocket(int * udpClientSocket,struct sockaddr_in * serverAddress);



#endif /*__UDP_CLIENT_H*/