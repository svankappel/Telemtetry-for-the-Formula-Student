
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(sta, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <nrfx_clock.h>

#include <errno.h>
#include <stdio.h>

#include <zephyr/posix/sys/socket.h>
#include <zephyr/posix/arpa/inet.h>
#include <zephyr/net/socket.h>
#include <unistd.h> 

#include "Task/wifi_sta.h"



int main(void)
{
	nrfx_clock_divider_set(NRF_CLOCK_DOMAIN_HFCLK, NRF_CLOCK_HFCLK_DIV_1);			//set 128 MHZ clock

	Task_Wifi_Sta_Init();
	//Task_UDP_Client_Init();

	k_msleep(10000);
	
	int udpClientSocket; 
	udpClientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if ( udpClientSocket < 0 ) {
		LOG_ERR( "UDP Client error: socket: %d\n", errno );
		k_sleep( K_FOREVER );
	}

	k_sleep( K_FOREVER );
	
}
