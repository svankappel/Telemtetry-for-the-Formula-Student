#include <zephyr/kernel.h>
#include <nrfx_clock.h>


#include "task/wifi_sta.h"
#include "task/udp_client.h"
#include "task/led.h"
#include "task/data_sender.h"
#include "task/memory_management.h"
#include "task/config_read.h"

K_HEAP_DEFINE(messageHeap,32768);
K_QUEUE_DEFINE(udpQueue);

K_TIMER_DEFINE(dataSenderTimer, data_Sender_timer_handler,NULL);

tSensor sensorBuffer[MAX_SENSORS];

int main(void)
{
	
	nrfx_clock_divider_set(NRF_CLOCK_DOMAIN_HFCLK, NRF_CLOCK_HFCLK_DIV_1);			//set 128 MHZ clock
	
	read_config();
	
	Task_Wifi_Sta_Init();
	Task_Led_Init();
	Task_UDP_Client_Init();

	Task_Data_Sender_Init();
	k_timer_start(&dataSenderTimer, K_SECONDS(1), K_MSEC(500));

	k_sleep( K_FOREVER );
	return 0;
}
