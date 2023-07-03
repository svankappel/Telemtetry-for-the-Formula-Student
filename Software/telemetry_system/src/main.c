#include <zephyr/kernel.h>
#include <nrfx_clock.h>


#include "task/wifi_sta.h"
#include "task/udp_client.h"
#include "task/led.h"
#include "task/data_sender.h"
#include "task/data_logger.h"
#include "task/memory_management.h"
#include "task/config_read.h"
#include "task/button_manager.h"

K_HEAP_DEFINE(messageHeap,32768);
K_QUEUE_DEFINE(udpQueue);

K_TIMER_DEFINE(dataSenderTimer, data_Sender_timer_handler,NULL);

K_TIMER_DEFINE(dataLoggerTimer, data_Logger_timer_handler,NULL);

tSensor sensorBuffer[MAX_SENSORS];

int main(void)
{
	
	nrfx_clock_divider_set(NRF_CLOCK_DOMAIN_HFCLK, NRF_CLOCK_HFCLK_DIV_1);			//set 128 MHZ clock
	
	Task_Led_Init();		//start led controller

	if(read_config() == 0)			//read configuration file ok
	{
		Task_Button_Manager_Init();		//start button manager

		Task_Wifi_Sta_Init();			//start wifi stationning
		
		Task_UDP_Client_Init();			//start udp client

		Task_Data_Sender_Init();		// start data sender
		k_timer_start(&dataSenderTimer, K_SECONDS(1), K_MSEC(500));

		Task_Data_Logger_Init();		//start data logger
		k_timer_start(&dataLoggerTimer, K_SECONDS(1), K_MSEC(50));
	}

	k_sleep( K_FOREVER );
	return 0;
}
