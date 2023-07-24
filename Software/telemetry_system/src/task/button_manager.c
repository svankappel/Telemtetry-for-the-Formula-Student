#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(button);
#include <nrfx_clock.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/reboot.h>

#include "button_manager.h"
#include "data_logger.h"

//! button thread  priority level
#define BUTTON_MANAGER_STACK_SIZE 1024
//! button thread  priority level
#define BUTTON_MANAGER_PRIORITY 8

//rebound delay
#define REBOUND_DELAY 50

//! button thread stack definition
K_THREAD_STACK_DEFINE(BUTTON_MANAGER_STACK, BUTTON_MANAGER_STACK_SIZE);
//! Variable to identify the button thread 
static struct k_thread buttonManagerThread;

//interrupt callbacks prototypes
void button_pressed(const struct device *dev, struct gpio_callback *cb,uint32_t pins);
void card_inserted(const struct device *dev, struct gpio_callback *cb,uint32_t pins);

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE	DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,{0});
static struct gpio_callback button_cb_data;

/*
 * Get sd card detect gpio configuration from the devicetree sddetect alias. This is mandatory.
 */
#define SD_DET_NODE	DT_ALIAS(sddetect)
#if !DT_NODE_HAS_STATUS(SD_DET_NODE, okay)
#error "Unsupported board: sd-detect devicetree alias is not defined"
#endif
static const struct gpio_dt_spec sd_det = GPIO_DT_SPEC_GET_OR(SD_DET_NODE, gpios,{0});
static struct gpio_callback sd_det_cb_data;


//button pressed and card inserted events
bool eventPressed;
bool eventCard;

//-----------------------------------------------------------------------------------------------------------------------
//button pressed interrupt callback
void button_pressed(const struct device *dev, struct gpio_callback *cb,uint32_t pins)
{
	eventPressed=true;		//event pressed
}

//-----------------------------------------------------------------------------------------------------------------------
//card inserted interrupt callback
void card_inserted(const struct device *dev, struct gpio_callback *cb,uint32_t pins)
{
	eventCard=true;		//event card inserted
}

//-----------------------------------------------------------------------------------------------------------------------
/*! Button_Manager implements the Button_Manager task
* @brief Button_Manager calls data logger button handler when a button is pressed
*        and reboot the system when an SD card is inserted
*/
void Button_Manager(void)
{
	//set events
	eventPressed=false;
	eventCard=false;

	//button
	gpio_pin_configure_dt(&button, GPIO_INPUT);								//configure pin
	gpio_pin_interrupt_configure_dt(&button,GPIO_INT_EDGE_TO_ACTIVE); 		//configure interrupt
	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));	//init callback
	gpio_add_callback(button.port, &button_cb_data);						//add callback

	//sd card detect pin
	gpio_pin_configure_dt(&sd_det, GPIO_INPUT);								//configure pin
	gpio_pin_interrupt_configure_dt(&sd_det,GPIO_INT_EDGE_TO_ACTIVE);		//configure interrupt
	gpio_init_callback(&sd_det_cb_data, card_inserted, BIT(sd_det.pin));	//init callback
	gpio_add_callback(sd_det.port, &sd_det_cb_data);						//add callback

	//thread infinite loop
	while (true) 
	{
		if(eventPressed)		//if interrupt of button were triggered
		{
			k_msleep(REBOUND_DELAY);		//wait some time
			if(gpio_pin_get_dt(&button)==1)		//check button state
				data_Logger_button_handler(); 	//call datalogger event handler
			eventPressed=false;				//reset event
		}

		if(eventCard)		//if interrupt of sd card were triggered
		{
			k_msleep(REBOUND_DELAY);		//wait some time
			if(gpio_pin_get_dt(&sd_det)==1)		//check gpio state
			{
				sys_reboot(0);		//reboot system
			}
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



