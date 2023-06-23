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
#define UDP_CLIENT_WAIT_TO_SEND_MS 1000

//! UDP Client stack definition
K_THREAD_STACK_DEFINE(UDP_CLIENT_STACK, UDP_CLIENT_STACK_SIZE);
//! Variable to identify the UDP Client thread
static struct k_thread udpClientThread;

//! UDP message sent by the client.
static const uint8_t udpTestMessage[] =
    "=============================UDP MESSAGE:=============================\n"
	"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque \n"
	"sodales lorem lorem, sed congue enim vehicula a. Sed finibus diam sed \n"
	"odio ultrices pharetra. Nullam dictum arcu ultricies turpis congue, \n"
	"vel venenatis turpis venenatis. Nam tempus arcu eros, ac congue libero \n"
	"tristique congue. Proin velit lectus, euismod sit amet quam in, \n"
	"maximus condimentum urna. Cras vel erat luctus, mattis orci ut, varius \n"
	"urna. Nam eu lobortis velit.\n"
	"\n"
	"Nullam sit amet diam vel odio sodales cursus vehicula eu arcu. Proin \n"
	"fringilla, enim nec consectetur mollis, lorem orci interdum nisi, \n"
	"vitae suscipit nisi mauris eu mi. Proin diam enim, mollis ac rhoncus \n"
	"vitae, placerat et eros. Suspendisse convallis, ipsum nec rhoncus \n"
	"aliquam, ex augue ultrices nisl, id aliquet mi diam quis ante. \n"
	"Pellentesque venenatis ornare ultrices. Quisque et porttitor lectus. \n"
	"Ut venenatis nunc et urna imperdiet porttitor non laoreet massa. Donec \n"
	"eleifend eros in mi sagittis egestas. Sed et mi nunc. Nunc vulputate, \n"
	"mauris non ullamcorper viverra, lorem nulla vulputate diam, et congue \n"
	"dui velit non erat. Duis interdum leo et ipsum tempor consequat. In \n"
	"faucibus enim quis purus vulputate nullam.\n"
	"\n";


/*! UDP_Client implements the UDP Client task.
* @brief UDP_Client uses a BSD socket to send messages to a defined UDP
*		server. 
*		A sample message will be sent to the IP address defined on the 
*		configuration file as Peer address.
* 		This function is used on an independent thread.
*/
void UDP_Client() {
    int udpClientSocket;
	struct sockaddr_in serverAddress;
	int connectionResult;
	int sentBytes = 0;
	
	// Starve the thread until a DHCP IP is assigned to the board 
	
    while( !context.connected ){
		k_msleep( UDP_CLIENT_SLEEP_TIME_MS );
	}

	// Server IPV4 address configuration 
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons( UDP_CLIENT_PORT );
    inet_pton(AF_INET, CONFIG_NET_CONFIG_PEER_IPV4_ADDR, &serverAddress.sin_addr );

	// Client socket creation 
	udpClientSocket = socket(serverAddress.sin_family, SOCK_DGRAM, IPPROTO_UDP );

	if ( udpClientSocket < 0 ) {
		LOG_ERR( "UDP Client error: socket: %d\n", errno );
		k_sleep( K_FOREVER );
	}

	// Connection to the server. 
	connectionResult = connect(udpClientSocket, ( struct sockaddr * )&serverAddress, sizeof( serverAddress ));

	if ( connectionResult < 0 ) {
		LOG_ERR( "UDP Client error: connect: %d\n", errno );
		k_sleep( K_FOREVER );
	}
	LOG_INF( "UDP Client connected correctly" );

	k_msleep( UDP_CLIENT_WAIT_TO_SEND_MS );

	while(true)
	{
		// Send the udp message 
		sentBytes = send(udpClientSocket, udpTestMessage, sizeof(udpTestMessage), 0);

		LOG_INF( "UDP Client mode. Sent: %d", sentBytes );
		if ( sentBytes < 0 ) 
		{
			LOG_ERR( "UDP Client error: send: %d\n", errno );
			close( udpClientSocket );
			LOG_ERR( "UDP Client error Connection from closed\n" );
		}
		k_msleep(500);
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



