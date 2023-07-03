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
* @brief Data Sender thread initialisation
*/
void Task_Data_Sender_Init( void );


#endif /*__DATA_SENDER_H*/