#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(sta, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>

#include <zephyr/posix/sys/socket.h>
#include <zephyr/posix/arpa/inet.h>
#include <zephyr/net/socket.h>
#include <unistd.h> 

#include "udp_client.h"
#include "deviceinformation.h"





//! Stack size for the UDP_SERVER thread
#define UDP_CLIENT_STACK_SIZE 2048
//! UDP_SERVER thread priority level
#define UDP_SERVER_PRIORITY 5
//! Time in miliseconds to wait to send the UDP message since the board 
// gets a stable IP address
#define UDP_CLIENT_WAIT_TO_SEND_MS 500

#define SOCKET_NUMBER 2

//! UDP client connection check interval in miliseconds
#define UDP_CLIENT_SLEEP_TIME_MS 100


//! UDP Client stack definition
K_THREAD_STACK_DEFINE(UDP_CLIENT_STACK, UDP_CLIENT_STACK_SIZE);
//! Variable to identify the UDP Client thread
static struct k_thread udpClientThread;







//! UDP message sent by the client.
static const uint8_t udpTestMessage[] =

	"{\n"
		"\"TensionBatteryHV\": 41,\n"
		"\"AmperageBatteryHV\": 39,\n"
		"\"TemperatureBatteryHV\": 32,\n"
		"\"EnginePower\": 44,\n"
		"\"EngineTemperature\": 33,\n"
		"\"EngineAngularSpeed\": 43,\n"
		"\"CarSpeed\": 44,\n"
		"\"PressureTireFL\": 38,\n"
		"\"PressureTireFR\": 38,\n"
		"\"PressureTireBL\": 39,\n"
		"\"PressureTireBR\": 43,\n"
		"\"InverterTemperature\": 34,\n"
		"\"TemperatureBatteryLV\": 43\n"
	"}"
	;




void connectUDPSocket(int * udpClientSocket,struct sockaddr_in * serverAddress)
{
	// Client socket creation 
	*udpClientSocket = socket(serverAddress->sin_family, SOCK_DGRAM, IPPROTO_UDP );

	if ( *udpClientSocket < 0 ) 
	{
		LOG_ERR( "UDP Client error: socket: %d\n", errno );
		k_sleep( K_FOREVER );
	}

	// Connection to the server. 
	int connectionResult = connect(*udpClientSocket, ( struct sockaddr * )serverAddress, sizeof( *serverAddress ));

	if ( connectionResult < 0 ) {
		LOG_ERR( "UDP Client error: connect: %d\n", errno );
		k_sleep( K_FOREVER );
	}
	LOG_INF( "UDP Client connected correctly" );
}



/*! UDP_Client implements the UDP Client task.
* @brief UDP_Client uses a BSD socket to send messages to a defined UDP
*		server. 
*		A sample message will be sent to the IP address defined on the 
*		configuration file as Peer address.
* 		This function is used on an independent thread.
*/
void UDP_Client() 
{
	char* addresses[SOCKET_NUMBER] = {"192.168.50.110","192.168.43.205"};
	int ports[SOCKET_NUMBER] = {1502,1502};


    int udpClientSocket[SOCKET_NUMBER];
	
	struct sockaddr_in serverAddress[SOCKET_NUMBER];

	int sentBytes[2];

	// Starve the thread until a DHCP IP is assigned to the board 
	while(!context.ip_assigned){
		k_msleep( UDP_CLIENT_SLEEP_TIME_MS );
	}

	for(int i=0;i<SOCKET_NUMBER;i++)
	{
		sentBytes[i]=0;

		// Server IPV4 address configuration 
		serverAddress[i].sin_family = AF_INET;
		serverAddress[i].sin_port = htons(ports[i]);
		inet_pton(AF_INET, addresses[i], &serverAddress[i].sin_addr );

		connectUDPSocket(&udpClientSocket[i],&serverAddress[i]);
	}

	k_msleep(UDP_CLIENT_WAIT_TO_SEND_MS);

	while(true)
	{
		//if wifi connection is lost
		if(!context.ip_assigned)
		{
			for(int i=0;i<SOCKET_NUMBER;i++)
				close(udpClientSocket[i]);		//close sockets


			// Starve the thread until a DHCP IP is assigned to the board 
			while( !context.ip_assigned ){
				k_msleep( UDP_CLIENT_SLEEP_TIME_MS );
			}

			
			for(int i=0;i<SOCKET_NUMBER;i++)
				connectUDPSocket(&udpClientSocket[i],&serverAddress[i]); // reconnect the socket

			k_msleep( UDP_CLIENT_WAIT_TO_SEND_MS );
		}
		else // if wifi connected
		{
			for(int i=0;i<SOCKET_NUMBER;i++)
			{
				// Send the udp message 
				sentBytes[i] = send(udpClientSocket[i], udpTestMessage, sizeof(udpTestMessage), 0);

				LOG_INF( "UDP %d Client mode. Sent: %d", i,sentBytes[i]);
				if ( sentBytes[i] < 0 ) 
				{
					LOG_ERR( "UDP %d Client error: send: %d\n", i,errno );
					close(udpClientSocket[i]);
					LOG_ERR( "UDP %d Client error Connection from closed\n",i );
				}
			}
			k_msleep(500);
		}
	}
	
}

/*! Task_UDP_Client_Init initializes the task UDP Client
*
* @brief UDP Client initialization
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
					UDP_SERVER_PRIORITY,									\
					0,														\
					K_NO_WAIT);	

	 k_thread_name_set(&udpClientThread, "udpClient");
	 k_thread_start(&udpClientThread);
}



