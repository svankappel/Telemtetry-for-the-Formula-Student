#ifndef __BUTTON_MANAGER_H
#define __BUTTON_MANAGER_H

/*! Button_Manager implements the Button_Manager task
* @brief Button_Manager call data logger button handler
*        when a button is pressed
*/
void Button_Manager(void);

/*! Task_Button_Manager_Init implements the Button_Manager task initialisation
* @brief Button_Manager thread initialisation
*      
*/
void Task_Button_Manager_Init( void );

//button press callback
void button_pressed(const struct device *dev, struct gpio_callback *cb,uint32_t pins);

#endif /*__BUTTON_MANAGER_H*/