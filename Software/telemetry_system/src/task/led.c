#include <nrfx_clock.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include "led.h"

#include "deviceinformation.h"
#include "data_logger.h"
#include "config_read.h"


//! Wifi thread priority level
#define LED_STACK_SIZE 1024
//! Wifi thread priority level
#define LED_PRIORITY 6

//! WiFi stack definition
K_THREAD_STACK_DEFINE(LED_STACK, LED_STACK_SIZE);
//! Variable to identify the Wifi thread
static struct k_thread ledThread;


/* 1000 msec = 1 sec */
#define LED_SLEEP_TIME_MS   100

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/* The devicetree node identifier for the "led1" alias. */
#define LED1_NODE DT_ALIAS(led1)

static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);


/*! Led implements the Led task
* @brief Led blink led1 when wifi is connected,
*        blink led2 when data logging is active
*		 and set led2 on when config file is failed
*/
void Led(void)
{
	int ret;

	if (!device_is_ready(led1.port)) 
		return;

	if (gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE) < 0) 
		return;

	if (!device_is_ready(led2.port)) 
		return;

	if (gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE) < 0) 
		return;

	while (true) {
		
		if (context.connected) 
			gpio_pin_toggle_dt(&led1);
		else 
			gpio_pin_set_dt(&led1, 0);


		if(configOK)
		{
			if (logEnable) 
			{
				gpio_pin_toggle_dt(&led2);
			}
			else 
			{
				gpio_pin_set_dt(&led2, 0);
			}
		}
		else
		{
			gpio_pin_set_dt(&led2, 1);
		}



		k_msleep(LED_SLEEP_TIME_MS);
	}
}


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



