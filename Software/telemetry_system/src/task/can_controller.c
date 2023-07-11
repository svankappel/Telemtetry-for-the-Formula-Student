#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(can);
#include <nrfx_clock.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>

#include "can_controller.h"
#include "memory_management.h"
#include "config_read.h"

#include <zephyr/random/rand32.h>

//! Wifi thread priority level
#define CAN_CONTROLLER_STACK_SIZE 4096
//! Wifi thread priority level
#define CAN_CONTROLLER_PRIORITY 2



//! WiFi stack definition
K_THREAD_STACK_DEFINE(CAN_CONTROLLER_STACK, CAN_CONTROLLER_STACK_SIZE);
//! Variable to identify the Wifi thread
static struct k_thread canControllerThread;

tSensor sensorBuffer[MAX_SENSORS];
K_MUTEX_DEFINE(sensorBufferMutex);

//-----------------------------------------------------------------------------------------------------------------------
/*! CAN_Controller implements the CAN_Controller task
* @brief CAN_Controller read the CAN Bus and fill the sensorBuffer array 
*        
*/
void CAN_Controller(void)
{
	
	//thread infinite loop
	while (true) 
	{
		//put some random datas in sensor buffer
		k_mutex_lock(&sensorBufferMutex,K_FOREVER);
		for(int i=0; i<configFile.sensorCount;i++)
		{
			sensorBuffer[i].value=sys_rand32_get()%100;
		}
		k_mutex_unlock(&sensorBufferMutex);
		
		k_msleep(50);			//loop delay
	}
	
}


//-----------------------------------------------------------------------------------------------------------------------
/*! Task_CAN_Controller_Init implements the CAN_Controller task initialization
* @brief CAN_Controller thread initialization
*      
*/
void Task_CAN_Controller_Init( void )
{
	k_thread_create	(&canControllerThread,
					CAN_CONTROLLER_STACK,										        
					CAN_CONTROLLER_STACK_SIZE,
					(k_thread_entry_t)CAN_Controller,
					NULL,
					NULL,
					NULL,
					CAN_CONTROLLER_PRIORITY,
					0,
					K_NO_WAIT);	

	 k_thread_name_set(&canControllerThread, "canController");
	 k_thread_start(&canControllerThread);
}



