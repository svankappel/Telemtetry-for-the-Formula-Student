#ifndef __DATA_SENDER_H
#define __DATA_SENDER_H

/*! Data_Sender implements the Data_Sender task
* @brief Data_Sender reads the data in the sensor buffer array and
*        creates the json string to send via Wi-Fi to the base 
*        station. This string is placed in the UDP_Client queue.
*/
void Data_Sender();


/*! Task_Data_Sender_Init initializes the task Data_Sender
*
* @brief Data Sender initialisation
*/
void Task_Data_Sender_Init( void );

/*! data_Sender_timer_handler is called by the timer interrupt
* @brief data_Sender_timer_handler submit a new work that call Data_Sender task     
*/
void data_Sender_timer_handler();


#endif /*__DATA_SENDER_H*/