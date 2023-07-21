#ifndef __BUTTON_MANAGER_H
#define __BUTTON_MANAGER_H

/*! Button_Manager implements the Button_Manager task
* @brief Button_Manager calls data logger button handler when a button is pressed
*        and reboot the system when an SD card is inserted
*/
void Button_Manager(void);

/*! Task_Button_Manager_Init implements the Button_Manager task initialisation
* @brief Button_Manager thread initialisation
*      
*/
void Task_Button_Manager_Init( void );


#endif /*__BUTTON_MANAGER_H*/