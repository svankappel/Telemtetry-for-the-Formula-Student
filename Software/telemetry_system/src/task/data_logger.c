#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(logger);

#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>
#include <zephyr/data/json.h>
#include <zephyr/device.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/data/json.h>
#include <zephyr/fs/fs.h>
#include <ff.h>

#include "data_logger.h"
#include "memory_management.h"
#include "deviceInformation.h"
#include "config_read.h"


K_WORK_DEFINE(dataLogWork, Data_Logger);		//dataLogWork -> called by timer to log data
K_WORK_DEFINE(startLog, data_log_start);		    //dataLogWork -> called by button
K_WORK_DEFINE(stopLog, data_log_stop);		    //dataLogWork -> called by button


//file system
static FATFS fat_fs;

// SD mounting info 
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};

// SD mount name
static const char *disk_mount_pt = "/SD:";


bool logEnable;             //log is recording variable
uint32_t timestamp;         //timestamp of the current data
int lineSize;               //line size in the csv file

//-----------------------------------------------------------------------------------------------------------------------
/*! Task_Data_Logger_Init initializes the task Data_Logger
*
* @brief Data Logger initialization
*/
void Task_Data_Logger_Init(void)
{
	logEnable=false;

    lineSize=10;          //size for timestamp
    for(int i=0;i<configFile.sensorNumber;i++)
    {
        if(strlen(sensorBuffer[i].name_log)<10)                 //if name is shorter than 10
            lineSize+=11;                                 // add max length of 32 bit variable in decimal + 1 for the ;
        else                                                //else
            lineSize+=(1+strlen(sensorBuffer[i].name_log));   // add string length of name + 1 for the ;
    }
}


//-----------------------------------------------------------------------------------------------------------------------
/*! Data_Logger implements the Data_Logger task
* @brief Data_Logger reads the data in the sensor buffer array and
*        creates the a line in the csv file on the SD card
*/
void Data_Logger() 
{
	if(logEnable)
    {
        //create line of csv file
        char str[lineSize];
        sprintf(str,"%d;",timestamp);
        for(int i=0;i<configFile.sensorNumber;i++)
        {
            sprintf(str,"%s%d;",str,sensorBuffer[i].value);
        }
        strcat(str,"\n");
        LOG_INF("%s",str);
        timestamp++;
    }
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
    //create file on sd card


    //create first line of csv file
    char str[lineSize];
    sprintf(str,"Timestamp;");
    for(int i=0;i<configFile.sensorNumber;i++)
    {
        sprintf(str,"%s%s;",str,sensorBuffer[i].name_log);
    }
    strcat(str,"\n");
    LOG_INF("%s",str);

    //set log enable to true
    logEnable=true;
    timestamp=0;
}

//-----------------------------------------------------------------------------------------------------------------------
//  stop log function -> called by button handler
void data_log_stop()
{
    //close file on sd card

    //set log enable to false
    logEnable=false;
}
