#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(gps);
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/uart.h>

#include "gps_controller.h"
#include "memory_management.h"
#include "config_read.h"


//! GPS thread priority level
#define GPS_CONTROLLER_STACK_SIZE 4096
//! GPS thread priority level
#define GPS_CONTROLLER_PRIORITY 2


//! GPS stack definition
K_THREAD_STACK_DEFINE(GPS_CONTROLLER_STACK, GPS_CONTROLLER_STACK_SIZE);
//! Variable to identify the GPS thread
static struct k_thread gpsControllerThread;

// change this to any other UART peripheral if desired 
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)

#define MSG_SIZE 85

// queue to store up to 10 messages (aligned to 4-byte boundary) 
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

// receive buffer used in UART ISR callback 
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

//serial callback function prototype
void serial_cb(const struct device *dev, void *user_data);


//gps fix
bool gpsFix;

//-----------------------------------------------------------------------------------------------------------------------
/*! GPS_Controller implements the GPS_Controller task
* @brief GPS_Controller read the UART form GPS and fill the sensorBuffer array 
*        
*/
void GPS_Controller(void)
{
	gpsFix = false;

	char rx_buf[MSG_SIZE];

	if (!device_is_ready(uart_dev)) 
	{
		LOG_INF("UART device not found!");
		return;
	}
	// configure interrupt and callback to receive data 
	uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
	uart_irq_rx_enable(uart_dev);

	// indefinitely wait for input from UART
	while (k_msgq_get(&uart_msgq, &rx_buf, K_FOREVER) == 0) 
	{

		//-----------------------------------------------------
		//		NMEA GSA Frame -	GPS receiver operating mod

		if(strstr(rx_buf,"$GNGSA"))
		{
			if(rx_buf[10]=='3')		//detect if gps has fix
				gpsFix=true;
			else
				gpsFix=false;
		}

		//analyse other frames only if GPS is fixed
		if(gpsFix)	
		{
			//-----------------------------------------------------
			//	NMEA GLL Frame - contains latitude and longitude

			if(strstr(rx_buf,"$GNGLL"))
			{
				
			}

			//-----------------------------------------------------
			//	NMEA RMC Frame - contains latitude and longitude

			if(strstr(rx_buf,"$GNRMC"))
			{
				
			}

			//-----------------------------------------------------
			//	NMEA GGA Frame - contains latitude and longitude

			if(strstr(rx_buf,"$GNGGA"))
			{
				
			}

			//-----------------------------------------------------
			//	NMEA VTG Frame - contains speed

			if(strstr(rx_buf,"$GNVTG"))
			{
				
			}
		}
	}
	
}


//-----------------------------------------------------------------------------------------------------------------------
/*! Task_GPS_Controller_Init implements the GPS_Controller task initialization
* @brief GPS_Controller thread initialization
*      
*/
void Task_GPS_Controller_Init( void )
{
	k_thread_create	(&gpsControllerThread,
					GPS_CONTROLLER_STACK,										        
					GPS_CONTROLLER_STACK_SIZE,
					(k_thread_entry_t)GPS_Controller,
					NULL,
					NULL,
					NULL,
					GPS_CONTROLLER_PRIORITY,
					0,
					K_NO_WAIT);	

	 k_thread_name_set(&gpsControllerThread, "gpsController");
	 k_thread_start(&gpsControllerThread);
}




//-----------------------------------------------------------------------------------------------------------------------
/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(uart_dev)) {
		return;
	}

	while (uart_irq_rx_ready(uart_dev)) {

		uart_fifo_read(uart_dev, &c, 1);

		if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
			// terminate string 
			rx_buf[rx_buf_pos] = '\0';

			// if queue is full, message is silently dropped 
			k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

			// reset the buffer (it was copied to the msgq) 
			rx_buf_pos = 0;
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}
		// else: characters beyond buffer size are dropped 
	}
}