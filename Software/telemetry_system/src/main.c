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
#include "task/can_controller.h"

K_HEAP_DEFINE(messageHeap,32768);
K_QUEUE_DEFINE(udpQueue);

K_TIMER_DEFINE(dataSenderTimer, data_Sender_timer_handler,NULL);

K_TIMER_DEFINE(dataLoggerTimer, data_Logger_timer_handler,NULL);


int main(void)
{
	Task_Led_Init();		//start led controller Thread
	Task_Button_Manager_Init();		//start button manager thread

	if(read_config() == 0)			//read configuration file ok
	{
		Task_Wifi_Sta_Init();			//start wifi stationning Thread
		
		Task_UDP_Client_Init();			//start udp client Thread

		Task_CAN_Controller_Init();		//start can controller thread

		Task_Data_Sender_Init();		// start data sender
		k_timer_start(&dataSenderTimer, K_SECONDS(0), K_MSEC((int)(1000/configFile.LiveFrameRate)));

		Task_Data_Logger_Init();		//start data logger
		k_timer_start(&dataLoggerTimer, K_SECONDS(0), K_MSEC((int)(1000/configFile.LogFrameRate)));
	}

	k_sleep( K_FOREVER );
	return 0;
}
