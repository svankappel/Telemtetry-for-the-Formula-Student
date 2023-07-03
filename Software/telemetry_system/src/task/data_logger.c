#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(sta, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>
#include <zephyr/data/json.h>
#include <zephyr/random/rand32.h>

#include "data_logger.h"
#include "memory_management.h"
#include "deviceInformation.h"
#include "config_read.h"


K_WORK_DEFINE(dataLogWork, Data_Logger);		//dataLogWork -> called by timer to log data
K_WORK_DEFINE(startLog, data_log_start);		    //dataLogWork -> called by button
K_WORK_DEFINE(stopLog, data_log_stop);		    //dataLogWork -> called by button

bool logEnable;



//-----------------------------------------------------------------------------------------------------------------------
/*! Task_Data_Logger_Init initializes the task Data_Logger
*
* @brief Data Logger initialisation
*/
void Task_Data_Logger_Init(void)
{
	logEnable=false;
}


//-----------------------------------------------------------------------------------------------------------------------
/*! Data_Logger implements the Data_Logger task
* @brief Data_Logger reads the data in the sensor buffer array and
*        creates the a line in the csv file on the SD card
*/
void Data_Logger() 
{
	
}

//-----------------------------------------------------------------------------------------------------------------------
/*! data_Logger_timer_handler is called by the timer interrupt
* @brief data_Logger_timer_handler submit a new work that call Data_Logger task     
*/
void data_Logger_timer_handler()
{
    k_work_submit(&dataLogWork);
}

//-----------------------------------------------------------------------------------------------------------------------
/*! data_Logger_button_handler is called by the button manager
* @brief data_Logger_button_handler submit a new work that call Data_Logger task     
*/
void data_Logger_button_handler()
{
    if(logEnable)
        k_work_submit(&stopLog);
    else
        k_work_submit(&startLog);
}


//-----------------------------------------------------------------------------------------------------------------------
//  start log function -> called by button handler
void data_log_start()
{
    logEnable=true;
}

//-----------------------------------------------------------------------------------------------------------------------
//  stop log function -> called by button handler
void data_log_stop()
{
    logEnable=false;
}
