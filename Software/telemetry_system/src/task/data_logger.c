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

//log file
struct fs_file_t logFile;



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

    lineSize=15;          //size for "Timestamp [ms];"
    for(int i=0;i<configFile.sensorCount;i++)
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
        sprintf(str,"%d;",timestamp);                       //timestamp at first column
        for(int i=0;i<configFile.sensorCount;i++)           //sensor values
        {
            sprintf(str,"%s%d;",str,sensorBuffer[i].value);
        }
        sprintf(str,"%s\n",str);                            // \n at end of line
        
        //write line in file
        fs_write(&logFile,str,strlen(str));

        timestamp+=(int)(1000/configFile.LogFrameRate);     //increment timestamp
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
    //mount sd card
    mp.mnt_point = disk_mount_pt;

	int res = fs_mount(&mp);	//mount sd card

	if (res != FR_OK) 		// return if mount failed
        return;

    struct fs_dir_t dirp;
	static struct fs_dirent entry;
    char fileName[9];
    int n = 0;

	fs_dir_t_init(&dirp);		//initialize directory

	// Verify fs_opendir() 			
	res = fs_opendir(&dirp, "/SD:");		//open SD card base directory
	if (res) 								//return error if open failed
		return;

	for (;;) 			//loop to find config file
	{
		res = fs_readdir(&dirp, &entry);			//read directory
        sprintf(fileName,"LOG_%04d",n);

		if (res || entry.name[0] == 0) 		//no more files
			break;						     //return

		if (entry.type == FS_DIR_ENTRY_FILE)		//file found
		{
			if(strstr(entry.name, fileName) != NULL)				//check if file is config file
                n++;
            else
                break;
		} 
	}

	fs_closedir(&dirp);	//close directory

    //init file object
    fs_file_t_init(&logFile);
    char path[18];
    sprintf(path,"/SD:/%s.csv",fileName);
    //create file on SD card
    res = fs_open(&logFile,path,FS_O_CREATE | FS_O_WRITE);
    if (res != 0) 		     // return if file creation failed
        return;

    //create first line of csv file
    char str[lineSize];

    sprintf(str,"Timestamp [ms];");     //timestamp at first column

    for(int i=0;i<configFile.sensorCount;i++)       //every sensors
    {
        sprintf(str,"%s%s;",str,sensorBuffer[i].name_log);
    }

    sprintf(str,"%s\n",str);                // \n at end of line

    //write in file
    res = fs_write(&logFile,str,strlen(str));
    if (res < 0) 		     // return if write failed
        return;

    //set log enable to true
    logEnable=true;
    timestamp=0;
}

//-----------------------------------------------------------------------------------------------------------------------
//  stop log function -> called by button handler
void data_log_stop()
{
    //set log enable to false
    logEnable=false;

    //close file on sd card
    fs_close(&logFile);

    //unmount disk
    fs_unmount(&mp);
}
