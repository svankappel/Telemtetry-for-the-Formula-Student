#include <nrfx_clock.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/reboot.h>

#include "button_manager.h"
#include "data_logger.h"

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE	DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,{0});
static struct gpio_callback button_cb_data;

/*
 * Get sd card detect gpio configuration from the devicetree sddetect alias. This is mandatory.
 */
#define SD_DET_NODE	DT_ALIAS(sddetect)
static const struct gpio_dt_spec sd_det = GPIO_DT_SPEC_GET_OR(SD_DET_NODE, gpios,{0});
static struct gpio_callback sd_det_cb_data;


static K_WORK_DELAYABLE_DEFINE(debounceBtn_work, debouncedBtn);


//-----------------------------------------------------------------------------------------------------------------------
//button pressed interrupt callback
void button_pressed(const struct device *dev, struct gpio_callback *cb,uint32_t pins)
{
	k_work_reschedule(&debounceBtn_work, K_MSEC(15));	//call debounced callback with a small delay, delay restarted after every bounces
}

//-----------------------------------------------------------------------------------------------------------------------
//button pressed debounced callback
void debouncedBtn(struct k_work *work)
{
    ARG_UNUSED(work);

	if(gpio_pin_get_dt(&button)==1)			//if button is pressed
		data_Logger_button_handler();		//call Data Logger button handler
}

//-----------------------------------------------------------------------------------------------------------------------
//card inserted or removed interrupt callback
void card_inserted(const struct device *dev, struct gpio_callback *cb,uint32_t pins)
{
	sys_reboot(0);			//reboot system 
}


//-----------------------------------------------------------------------------------------------------------------------
/*! Task_Button_Manager_Init implements the Button_Manager task initialization
* @brief Button_Manager interrupts initialization for the button and the detect pin of the SD card slot
*      
*/
void Task_Button_Manager_Init( void )
{
	//button
	gpio_pin_configure_dt(&button, GPIO_INPUT);								//configure pin
	gpio_pin_interrupt_configure_dt(&button,GPIO_INT_EDGE_BOTH); 		//configure interrupt
	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));	//init callback
	gpio_add_callback(button.port, &button_cb_data);						//add callback

	//sd card detect pin
	gpio_pin_configure_dt(&sd_det, GPIO_INPUT);								//configure pin
	gpio_pin_interrupt_configure_dt(&sd_det,GPIO_INT_EDGE_BOTH);		//configure interrupt
	gpio_init_callback(&sd_det_cb_data, card_inserted, BIT(sd_det.pin));	//init callback
	gpio_add_callback(sd_det.port, &sd_det_cb_data);						//add callback
}



