#include <zephyr/kernel.h>
#include <nrfx_clock.h>


#include "task/wifi_sta.h"
#include "task/udp_client.h"
#include "task/led.h"
#include "task/data_sender.h"
#include "task/memory_management.h"

K_HEAP_DEFINE(memHeap,4096);
K_QUEUE_DEFINE(udpQueue);


int main(void)
{
	nrfx_clock_divider_set(NRF_CLOCK_DOMAIN_HFCLK, NRF_CLOCK_HFCLK_DIV_1);			//set 128 MHZ clock

	Task_Wifi_Sta_Init();
	Task_Led_Init();
	Task_UDP_Client_Init();
	Task_Data_Sender_Init();

	k_sleep( K_FOREVER );
	return 0;
}
