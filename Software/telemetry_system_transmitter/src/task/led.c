/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file led.c
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief Led task controlls leds. The led1 blinks if the wifi is 
 *        connected and led2 blinks if logs are currently recording. 
 *        if an error occured during the configuration file read, the 
 *        led2 stays on.
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
#include <nrfx_clock.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include "led.h"

//project files includes
#include "deviceinformation.h"
//#include "data_logger.h"
#include "config_read.h"


//! LED thread priority level
#define LED_STACK_SIZE 1024
//! LED thread priority level
#define LED_PRIORITY 7

//! LED stack definition
K_THREAD_STACK_DEFINE(LED_STACK, LED_STACK_SIZE);
//! Variable to identify the LED thread
static struct k_thread ledThread;


//blink time
#define LED_SLEEP_TIME_MS   100

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/* The devicetree node identifier for the "led1" alias. */
#define LED1_NODE DT_ALIAS(led1)

static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);


//-----------------------------------------------------------------------------------------------------------------------
/*! Led implements the Led task
* @brief Led blink led1 when wifi is connected,
*        blink led2 when data logging is active
*		 and set led2 on when config file is failed
*/
void Led(void)
{
	//check if led 1 is ready
	if (!device_is_ready(led1.port)) 
		return;

	//configure gpio
	if (gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE) < 0) 
		return;

	//check if led 2 is ready
	if (!device_is_ready(led2.port)) 
		return;

	//configure gpio
	if (gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE) < 0) 
		return;


	while (true) //--------------------------------------------------- thread infinite loop
	{
		//blink led 1 if wifi is connected
		if (context.connected) 	
			gpio_pin_toggle_dt(&led1);
		else 
			gpio_pin_set_dt(&led1, 0);


		//blink led 2 if datalogger is recording
		if(configOK)
		{
			/*
			if (logEnable) 
				gpio_pin_toggle_dt(&led2);
			else 
				gpio_pin_set_dt(&led2, 0);
			*/
		}
		else	//set led on if config file is not ok
		{
			gpio_pin_set_dt(&led2, 1);
		}

		k_msleep(LED_SLEEP_TIME_MS);
	}	//----------------------------------------------------------- end of thread infinite loop
}

//-----------------------------------------------------------------------------------------------------------------------
/*! Task_Led_Init implements the Led task initialization
* @brief Led thread initialization
*/
void Task_Led_Init( void )
{
	k_thread_create	(&ledThread,
					LED_STACK,										        
					LED_STACK_SIZE,
					(k_thread_entry_t)Led,
					NULL,
					NULL,
					NULL,
					LED_PRIORITY,
					0,
					K_NO_WAIT);	

	 k_thread_name_set(&ledThread, "ledBlinking");
	 k_thread_start(&ledThread);
}
