/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file udp_client.c
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief UDP Client task reads the messages in the udp Queue and send
 *        them to the server(s)
 * ---------------------------------------------------------------------
 * Telemetry system for the Valais Wallis Racing Team.
 * This file contains code for the onboard device of the telemetry
 * system. The system receives the data from the sensors on the CAN bus 
 * and the data from the GPS on a UART port. An SD Card contains a 
 * configuration file with all the system parameters. The measurements 
 * are sent via Wi-Fi to a computer on the base station. The measurements 
 * are also saved in a CSV file on the SD card. 
 *--------------------------------------------------------------------*/

//includes
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(udp);
#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>
#include <zephyr/posix/sys/socket.h>
#include <zephyr/posix/arpa/inet.h>
#include <zephyr/net/socket.h>
#include <unistd.h> 

//project file includes
#include "udp_client.h"
#include "deviceinformation.h"
#include "memory_management.h"
#include "config_read.h"

//! Stack size for the UDP_SERVER thread
#define UDP_CLIENT_STACK_SIZE 2048
//! UDP_SERVER thread priority level
#define UDP_CLIENT_PRIORITY 3
//! Time in miliseconds to wait to send the UDP message since the board 
// gets a stable IP address
#define UDP_CLIENT_WAIT_TO_SEND_MS 500


//! UDP Client stack definition
K_THREAD_STACK_DEFINE(UDP_CLIENT_STACK, UDP_CLIENT_STACK_SIZE);
//! Variable to identify the UDP Client thread
static struct k_thread udpClientThread;


//-----------------------------------------------------------------------------------------------------------------------
/*! UDP_Client implements the UDP Client task.
* @brief UDP_Client uses BSD sockets to send messages to one or multiple UDP
*		server(s). 
*		The thread send to the UDP servers all the data pushed on the udpQueue.
*/
void UDP_Client() 
{
	//get number of servers the system needs to send data
	int socketCount = configFile.serverCount;

	//addresses of the sockets
	char* addresses[socketCount];
	//ports of the sockets
	int ports[socketCount];

	for(int i=0;i<socketCount;i++)		//loop for all sockets
	{
		addresses[i]=configFile.Server[i].address;		//assign address
		ports[i]=configFile.Server[i].port;				//assign port
	}
	
    int udpClientSocket[socketCount];					//client sockets
	struct sockaddr_in serverAddress[socketCount];		//servers addresses for all sockets
	int sentBytes[socketCount];						//sent bytes variable for all sockets


	// stop the thread until a DHCP IP is assigned to the board 
	while(!context.ip_assigned)
	{
		//delete messages coming from queue while ip is not assigned
		char * memPtr;										//message to get from queue
		memPtr = k_queue_get(&udpQueue,K_FOREVER);			//wait for message in queue
		k_heap_free(&messageHeap,memPtr);					//free memory allocation made in data sender
	}

	for(int i=0;i<socketCount;i++)		//loop for all sockets
	{
		sentBytes[i]=0;					//set sent byte variable

		// Server IPV4 address configuration 
		serverAddress[i].sin_family = AF_INET;
		serverAddress[i].sin_port = htons(ports[i]);
		inet_pton(AF_INET, addresses[i], &serverAddress[i].sin_addr );

		//connect UDP socket
		connectUDPSocket(&udpClientSocket[i],&serverAddress[i]);
	}

	k_msleep(UDP_CLIENT_WAIT_TO_SEND_MS); //wait some time before sending messages

	while(true)	// --------------------------------------------------------------------Thread infinite loop
	{

		if(!context.ip_assigned) 	// ------------------------------------ if wifi connection is lost
		{
			//close all sockets
			for(int i=0;i<socketCount;i++)
				close(udpClientSocket[i]);		

			// stop the thread until a DHCP IP is assigned to the board 
			while( !context.ip_assigned )
			{
				//delete messages coming from queue while ip is not assigned
				char * memPtr;										//message to get from queue
				memPtr = k_queue_get(&udpQueue,K_FOREVER);			//wait for message in queue
				k_heap_free(&messageHeap,memPtr);					//free memory allocation made in data sender
			}
			
			// reconnect all sockets
			for(int i=0;i<socketCount;i++)
				connectUDPSocket(&udpClientSocket[i],&serverAddress[i]); 

			//wait some time before sending messages
			k_msleep( UDP_CLIENT_WAIT_TO_SEND_MS );
		}
		else // ----------------------------------------------------------------- if wifi is connected 
		{
			//-----------------------------------------
			//			Receive data from queue
			
			char udpMessage[udpQueueMesLength];				//message to send
			char * memPtr;										//message to get from queue
			memPtr = k_queue_get(&udpQueue,K_FOREVER);			//wait for message in queue
			int size = 0;										//size of message
			for(int i=0; i<udpQueueMesLength; i++)				//loop to copy message
			{
				udpMessage[i]=memPtr[i];				//copy char
				if(udpMessage[i]=='}')					//if the loop reached the end of the JSON message
				{
					size = i+1;							//calculate size of the message
					break;								//break loop
				}
			}

			k_heap_free(&messageHeap,memPtr);		//free memory allocation made in data sender


			//------------------------------------------
			//		send data to socket(s)

			for(int i=0;i<socketCount;i++)		//loop for all sockets
			{
				// Send the udp message 
				sentBytes[i] = send(udpClientSocket[i], udpMessage, size, 0);

				//LOG_INF( "UDP %d Client mode. Sent: %d", i,sentBytes[i]);		//log message
				
				if ( sentBytes[i] < 0 ) 		//in case of error
				{
					LOG_ERR( "UDP %d Client error: send: %d\n", i,errno );		//log message
					close(udpClientSocket[i]);									//close socket
					LOG_ERR( "UDP %d Client error Connection from closed\n",i );
				}
			}
		}
	}	// ------------------------------------------------------------------------------  end of thread infinite loop
}



//------------------------------------------------------------------------------------------------
/*! @brief UDP socket connect function
 *  @param udpClientSocket UDP Client socket variable
 *  @param serverAdress Socket Address struct
 */
void connectUDPSocket(int * udpClientSocket,struct sockaddr_in * serverAddress)
{
	// Client socket creation 
	*udpClientSocket = socket(serverAddress->sin_family, SOCK_DGRAM, IPPROTO_UDP );

	// Connection to the server. 
	int connectionResult = connect(*udpClientSocket, ( struct sockaddr * )serverAddress, sizeof( *serverAddress ));

	if ( connectionResult < 0 ) {
		LOG_ERR( "UDP Client error: connect: %d\n", errno );
		k_sleep( K_FOREVER );
	}
	LOG_INF( "UDP Client connected correctly" );
}

//-----------------------------------------------------------------------------------------------------------------------
/*! Task_UDP_Client_Init initializes the task UDP Client
*
* @brief 
*/
void Task_UDP_Client_Init( void ){
	k_thread_create	(														\
					&udpClientThread,										\
					UDP_CLIENT_STACK,										\
					UDP_CLIENT_STACK_SIZE,									\
					(k_thread_entry_t)UDP_Client,							\
					NULL,													\
					NULL,													\
					NULL,													\
					UDP_CLIENT_PRIORITY,									\
					0,														\
					K_NO_WAIT);	

	 k_thread_name_set(&udpClientThread, "udpClient");
	 k_thread_start(&udpClientThread);
}