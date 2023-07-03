#ifndef __DATA_LOGGER_H
#define __DATA_LOGGER_H

/*! Data_Logger implements the Data_Logger task
* @brief Data_Logger reads the data in the sensor buffer array and
*        creates the a line in the csv file on the SD card
*/
void Data_Logger();

/*! Task_Data_Logger_Init initializes the task Data_Logger
*
* @brief Data Logger initialisation
*/
void Task_Data_Logger_Init( void );

/*! data_Logger_timer_handler is called by the timer interrupt
* @brief data_Logger_timer_handler submit a new work that call Data_Logger task     
*/
void data_Logger_timer_handler();

/*! data_Logger_button_handler is called by the button manager
* @brief data_Logger_button_handler submit a new work that call Data_Logger task     
*/
void data_Logger_button_handler();


//function prototypes
void data_log_start();
void data_log_stop();

//logEnable accessible in other files
extern bool logEnable;

#endif /*__DATA_LOGGER_H*/