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
K_MUTEX_DEFINE(sensorBufferMutex);

int main(void)
{
	Task_Led_Init();		//start led controller

	if(read_config() == 0)			//read configuration file ok
	{
		Task_Button_Manager_Init();		//start button manager

		Task_Wifi_Sta_Init();			//start wifi stationning
		
		Task_UDP_Client_Init();			//start udp client

		Task_Data_Sender_Init();		// start data sender
		k_timer_start(&dataSenderTimer, K_SECONDS(0), K_MSEC((int)(1000/configFile.LiveFrameRate)));

		Task_Data_Logger_Init();		//start data logger
		k_timer_start(&dataLoggerTimer, K_SECONDS(0), K_MSEC((int)(1000/configFile.LogFrameRate)));
	}

	k_sleep( K_FOREVER );
	return 0;
}
