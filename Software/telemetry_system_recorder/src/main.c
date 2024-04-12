/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file main.c
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief Main of Telemetry system. Starts all tasks of the program
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
#include <zephyr/kernel.h>
#include <nrfx_clock.h>

// include all tasks files
//#include "task/wifi_sta.h"
//#include "task/udp_client.h"
#include "task/led.h"
//#include "task/data_sender.h"
#include "task/data_logger.h"
#include "task/memory_management.h"
#include "task/config_read.h"
#include "task/button_manager.h"
#include "task/can_controller.h"
//#include "task/gps_controller.h"


//message heap for udp queue
K_HEAP_DEFINE(messageHeap,32768);

//udp queue
K_QUEUE_DEFINE(udpQueue);


//--------------------------------------------------------------------------
//			main
int main(void)
{
	Task_Led_Init();				//start led controller Thread
	Task_Button_Manager_Init();		//start button manager

	int ret = read_config();		//read configuration file

	if(ret == 0)			//configuration file ok
	{
//		Task_Wifi_Sta_Init();			//start wifi stationning Thread
		
//		Task_UDP_Client_Init();			//start udp client Thread

//		Task_GPS_Controller_Init();		//start gps controller thread

//		Task_Data_Sender_Init();		// start data sender

		Task_Data_Logger_Init();		//start data logger

		Task_CAN_Controller_Init();		//start can controller thread

		k_sleep(K_SECONDS(7));
		if(configFile.RecordOnStart)                            //if record autostart is selected
        	data_Logger_button_handler_start();
	}

	k_sleep( K_FOREVER );
	return 0;
}
