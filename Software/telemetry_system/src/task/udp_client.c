#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(sta, LOG_LEVEL_DBG);


#define CONFIG_NET_CONFIG_PEER_IPV4_ADDR1 "192.168.50.110"
#define CONFIG_NET_CONFIG_PEER_IPV4_ADDR2 "192.168.43.205"

#define UDP_CLIENT_PORT1 1502
#define UDP_CLIENT_PORT2 1502

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
#define UDP_CLIENT_WAIT_TO_SEND_MS 1000



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
void UDP_Client() {
    int udpClientSocket1;
	int udpClientSocket2;
	
	struct sockaddr_in serverAddress1;
	struct sockaddr_in serverAddress2;

	int sentBytes1 = 0;
	int sentBytes2 = 0;
	
	// Starve the thread until a DHCP IP is assigned to the board 
	while(!context.ip_assigned){
		k_msleep( UDP_CLIENT_SLEEP_TIME_MS );
	}

	// Server IPV4 address configuration 
	serverAddress1.sin_family = AF_INET;
	serverAddress1.sin_port = htons( UDP_CLIENT_PORT1 );
	inet_pton(AF_INET, CONFIG_NET_CONFIG_PEER_IPV4_ADDR1, &serverAddress1.sin_addr );

	// Client socket creation 
	udpClientSocket1 = socket(serverAddress1.sin_family, SOCK_DGRAM, IPPROTO_UDP );

	if ( udpClientSocket1 < 0 ) {
		LOG_ERR( "UDP Client error: socket: %d\n", errno );
		k_sleep( K_FOREVER );
	}

	// Server IPV4 address configuration 
	serverAddress2.sin_family = AF_INET;
	serverAddress2.sin_port = htons( UDP_CLIENT_PORT2 );
	inet_pton(AF_INET, CONFIG_NET_CONFIG_PEER_IPV4_ADDR2, &serverAddress2.sin_addr );

	// Client socket creation 
	udpClientSocket2 = socket(serverAddress2.sin_family, SOCK_DGRAM, IPPROTO_UDP );

	if ( udpClientSocket2 < 0 ) {
		LOG_ERR( "UDP Client error: socket: %d\n", errno );
		k_sleep( K_FOREVER );
	}

	k_msleep( UDP_CLIENT_WAIT_TO_SEND_MS );

	//connect socket
	connectUDPSocket(&udpClientSocket1,&serverAddress1);
	connectUDPSocket(&udpClientSocket2,&serverAddress2);

	while(true)
	{
		//if wifi connection is lost
		if(!context.ip_assigned)
		{
			close(udpClientSocket1);		//close socket
			close(udpClientSocket2);		//close socket

			// Starve the thread until a DHCP IP is assigned to the board 
			while( !context.ip_assigned ){
				k_msleep( UDP_CLIENT_SLEEP_TIME_MS );
			}

			k_msleep( UDP_CLIENT_WAIT_TO_SEND_MS );

			connectUDPSocket(&udpClientSocket1,&serverAddress1); // reconnect the socket
			connectUDPSocket(&udpClientSocket2,&serverAddress2); // reconnect the socket
		}
		else // if wifi connected
		{
			// Send the udp message 
			sentBytes1 = send(udpClientSocket1, udpTestMessage, sizeof(udpTestMessage), 0);
			sentBytes2 = send(udpClientSocket2, udpTestMessage, sizeof(udpTestMessage), 0);

			LOG_INF( "UDP 1 Client mode. Sent: %d", sentBytes1);
			if ( sentBytes1 < 0 ) 
			{
				LOG_ERR( "UDP 1 Client error: send: %d\n", errno );
				close(udpClientSocket1);
				LOG_ERR( "UDP 1 Client error Connection from closed\n" );
			}

			LOG_INF( "UDP 2 Client mode. Sent: %d", sentBytes2);
			if ( sentBytes2 < 0 ) 
			{
				LOG_ERR( "UDP 2 Client error: send: %d\n", errno );
				close(udpClientSocket2);
				LOG_ERR( "UDP 2 Client error Connection from closed\n" );
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



