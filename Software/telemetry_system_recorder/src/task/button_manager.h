/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file button_manager.h
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief button manager task call button handler function from data
 * 		  logger when the button interruption is triggered. The button 
 * 		  manager includes a debouncer. This task also handle the SD
 * 		  card detect input. When the card is inserted or removed, the
 * 		  system is restarted
 * ---------------------------------------------------------------------
 * Telemetry system for the Valais Wallis Racing Team.
 * This file contains code for the onboard device of the telemetry
 * system. The system receives the data from the sensors on the CAN bus 
 * and the data from the GPS on a UART port. An SD Card contains a 
 * configuration file with all the system parameters. The measurements 
 * are sent via Wi-Fi to a computer on the base station. The measurements 
 * are also saved in a CSV file on the SD card. 
 *--------------------------------------------------------------------*/

#ifndef __BUTTON_MANAGER_H
#define __BUTTON_MANAGER_H

/*! Task_Button_Manager_Init implements the Button_Manager task initialization
* @brief Button_Manager interrupts initialization for the button and the detect pin of the SD card slot
*      
*/
void Task_Button_Manager_Init( void );

/*! @brief button pressed interrupt callback */
void button_pressed(const struct device *dev, struct gpio_callback *cb,uint32_t pins);

/*! @brief card inserted or removed interrupt callback */
void card_inserted(const struct device *dev, struct gpio_callback *cb,uint32_t pins);

/*! @brief button pressed debounced callback */
void debouncedBtn(struct k_work *work);

#endif /*__BUTTON_MANAGER_H*/