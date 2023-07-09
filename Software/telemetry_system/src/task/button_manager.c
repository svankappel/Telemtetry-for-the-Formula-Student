#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(button);
#include <nrfx_clock.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>


#include "button_manager.h"
#include "data_logger.h"

//! Wifi thread priority level
#define BUTTON_MANAGER_STACK_SIZE 1024
//! Wifi thread priority level
#define BUTTON_MANAGER_PRIORITY 8

//rebound delay
#define REBOUND_DELAY 50

//! WiFi stack definition
K_THREAD_STACK_DEFINE(BUTTON_MANAGER_STACK, BUTTON_MANAGER_STACK_SIZE);
//! Variable to identify the Wifi thread
static struct k_thread buttonManagerThread;

//button press callback
void button_pressed(const struct device *dev, struct gpio_callback *cb,uint32_t pins);
/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE	DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,{0});
static struct gpio_callback button_cb_data;

bool eventPressed;

//-----------------------------------------------------------------------------------------------------------------------
//button press callback
void button_pressed(const struct device *dev, struct gpio_callback *cb,uint32_t pins)
{
	eventPressed=true;		//event pressed
}

//-----------------------------------------------------------------------------------------------------------------------
/*! Button_Manager implements the Button_Manager task
* @brief Button_Manager call data logger button handler
*        when a button is pressed
*/
void Button_Manager(void)
{
	eventPressed=false;

	//check if device is ready
	if (!device_is_ready(button.port))
		return;

	//configure pin
	if (gpio_pin_configure_dt(&button, GPIO_INPUT) != 0)
		return;
	
	//configure interrupt
	if (gpio_pin_interrupt_configure_dt(&button,GPIO_INT_EDGE_TO_ACTIVE) != 0) 
		return;

	//add callback
	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);

	//thread infinite loop
	while (true) 
	{
		if(eventPressed)		//if interrupt were triggered
		{
			k_msleep(REBOUND_DELAY);		//wait some time
			if(gpio_pin_get_dt(&button)==1)		//check button state
				data_Logger_button_handler(); 	//call datalogger event handler
			eventPressed=false;				//reset event
		}
		
		k_msleep(100);			//loop delay
	}
	
}


//-----------------------------------------------------------------------------------------------------------------------
/*! Task_Button_Manager_Init implements the Button_Manager task initialization
* @brief Button_Manager thread initialization
*      
*/
void Task_Button_Manager_Init( void )
{
	k_thread_create	(&buttonManagerThread,
					BUTTON_MANAGER_STACK,										        
					BUTTON_MANAGER_STACK_SIZE,
					(k_thread_entry_t)Button_Manager,
					NULL,
					NULL,
					NULL,
					BUTTON_MANAGER_PRIORITY,
					0,
					K_NO_WAIT);	

	 k_thread_name_set(&buttonManagerThread, "buttonManager");
	 k_thread_start(&buttonManagerThread);
}



