#include <nrfx_clock.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>

#include "deviceinformation.h"


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
/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);


/*
*  toggle led thread
*  
*/
void Led(void)
{
	int ret;

	if (!device_is_ready(led.port)) 
	{
		return;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) 
	{
		return;
	}

	while (true) {
		
		if (context.connected) 
		{
			gpio_pin_toggle_dt(&led);
			k_msleep(LED_SLEEP_TIME_MS);
		} 
		else 
		{
			gpio_pin_set_dt(&led, 0);
			k_msleep(LED_SLEEP_TIME_MS);
		}
		
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



