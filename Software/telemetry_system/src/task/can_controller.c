#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(can);
#include <nrfx_clock.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/can.h>

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


const struct can_filter filter = {
        .flags = CAN_FILTER_DATA | CAN_FILTER_IDE,
        .id = 0x123,
        .mask = CAN_STD_ID_MASK
};

CAN_MSGQ_DEFINE(can_rx_msgq,2);

struct can_frame rx_frame;
int filter_id;
const struct device * can_dev;

//-----------------------------------------------------------------------------------------------------------------------
/*! CAN_Controller implements the CAN_Controller task
* @brief CAN_Controller read the CAN Bus and fill the sensorBuffer array 
*        
*/
void CAN_Controller(void)
{
	
	can_dev = device_get_binding("CAN_1");
	if (!can_dev) {
		printk("CAN: Device driver not found.\n");
		return;
	}

	filter_id = can_add_rx_filter_msgq(can_dev, &can_rx_msgq, &filter);
	if (filter_id < 0) 
	{
		LOG_ERR("Unable to add rx msgq [%d]", filter_id);
		return;
	}

	//thread infinite loop
	while (true) 
	{
		k_msgq_get(&can_rx_msgq, &rx_frame, K_FOREVER);

		LOG_INF("coucou");
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



