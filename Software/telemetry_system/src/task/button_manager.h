#ifndef __BUTTON_MANAGER_H
#define __BUTTON_MANAGER_H

/*! Task_Button_Manager_Init implements the Button_Manager task initialization
* @brief Button_Manager interrupts initialization for the button and the detect pin of the SD card slot
*      
*/
void Task_Button_Manager_Init( void );

//functions prototypes
void button_pressed(const struct device *dev, struct gpio_callback *cb,uint32_t pins);
void card_inserted(const struct device *dev, struct gpio_callback *cb,uint32_t pins);
void debouncedBtn(struct k_work *work);

#endif /*__BUTTON_MANAGER_H*/