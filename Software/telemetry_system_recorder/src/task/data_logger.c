/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file data_logger.c
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief Data logger task. This task reads periodically the sensor 
 *        buffer and puts the data in the CSV file
 * ---------------------------------------------------------------------
 * Telemetry system for the Valais Wallis Racing Team.
 * This file contains code for the onboard device of the telemetry
 * system. The system receives the data from the sensors on the CAN bus 
 * and the data from the GPS on a UART port. An SD Card contains a 
 * configuration file with all the system parameters. The measurements 
 * are sent via Wi-Fi to a computer on the base station. The measurements 
 * are also saved in a CSV file on the SD card. 
 *--------------------------------------------------------------------*/

//includes
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
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>

//includes of project files
#include "data_logger.h"
#include "memory_management.h"
//#include "deviceInformation.h"
#include "config_read.h"

// Non Volatile Strorage (NVS) defines
static struct nvs_fs fs;
#define NVS_PARTITION		storage_partition
#define NVS_PARTITION_DEVICE	FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET	FIXED_PARTITION_OFFSET(NVS_PARTITION)

#define LOGNAME_ID 1


//periodic timer that reads measurements
K_TIMER_DEFINE(dataLoggerTimer, data_Logger_timer_handler,NULL);

//works for processes triggerd by interruptions
K_WORK_DEFINE(dataLogWork, Data_Logger);		//dataLogWork -> called by timer to log data
K_WORK_DEFINE(startLog, data_log_start);		//start log -> called by button
K_WORK_DEFINE(stopLog, data_log_stop);		    //stop log -> called by button

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

// global variables
bool logEnable;             //log is recording variable
uint32_t timestamp;         //timestamp of the current data
int lineSize;               //line size in the csv file

//recording status callback function pointers
void (*recordON)();
void (*recordOFF)();

//-----------------------------------------------------------------------------------------------------------------------
/*! Data_Logger implements the Data_Logger task
* @brief Data_Logger reads the data in the sensor buffer array and
*        creates the a line in the csv file on the SD card
*/
void Data_Logger() 
{
	if(logEnable)       //if logging is enabled
    {
        //------------------------------------------------------------  create line of csv file

        char str[lineSize];
        sprintf(str,"%d;",timestamp);                       //print timestamp at first column of CSV file

        k_mutex_lock(&sensorBufferMutex,K_FOREVER);		    //lock sensor buffer mutex

        for(int i=0;i<configFile.sensorCount;i++)           //print sensor values in CSV file
        {
            sprintf(str,"%s%u;",str,sensorBuffer[i].value);
        }

        k_mutex_unlock(&sensorBufferMutex);		            //unlock sensor buffer mutex

        k_mutex_lock(&gpsBufferMutex,K_FOREVER);		    //lock gps buffer mutex

        
        sprintf(str,"%s%d.%d %s%d.%d",gpsBuffer.lat_sign==1?"":"-",gpsBuffer.lat_characteristic,gpsBuffer.lat_mantissa,
                                      gpsBuffer.long_sign==1?"":"-",gpsBuffer.long_characteristic, gpsBuffer.long_mantissa);
        sprintf(str,"%s%s;",str,gpsBuffer.speed);
        sprintf(str,"%s%s;",str,gpsBuffer.fix ? "true" : "false");
        
        k_mutex_unlock(&gpsBufferMutex);		            //unlock gps buffer mutex


        sprintf(str,"%s\n",str);                            //append \n at end of line of the CSV file
        

        //--------------------------------------------------------------  write line in file

        if(fs_write(&logFile,str,strlen(str))<0)            //write string in file
            k_work_submit(&stopLog);                        //stop log in case of error

        //-------------------------------------------------------------- increment timestamp

        timestamp+=(int)(1000/configFile.LogFrameRate);     
    }     
}

//-----------------------------------------------------------------------------------------------------------------------
/*! set recording status callbacks
* @brief set recording status callbacks to inform the CAN to the status of the datalogger
*/
void set_RecordingStatus_callbacks(void (*recON)(), void (*recOFF)())
{
    recordON = recON;
    recordOFF = recOFF;
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
    if(logEnable)                   //if system is currently recording logs
        k_work_submit(&stopLog);        //stop
    else
        k_work_submit(&startLog);       //start
}

//-----------------------------------------------------------------------------------------------------------------------
/*! data_Logger_button_handler is called by the button manager
* @brief data_Logger_button_handler submit a new work that call Data_Logger task     
*/
void data_Logger_button_handler_start()
{
    if(!logEnable)                   //if system is not currently recording logs
        k_work_submit(&startLog);       //start
}

//-----------------------------------------------------------------------------------------------------------------------
/*! data_Logger_button_handler is called by the button manager
* @brief data_Logger_button_handler submit a new work that call Data_Logger task     
*/
void data_Logger_button_handler_stop()
{
    if(logEnable)                   //if system is currently recording logs
        k_work_submit(&stopLog);        //stop
}


//-----------------------------------------------------------------------------------------------------------------------
//  start log function -> called by button handler
void data_log_start()
{
    //---------------------------------------------- set recording status on the can
    (*recordON)();
    //---------------------------------------------- mount sd card
    mp.mnt_point = disk_mount_pt;

	int res = fs_mount(&mp);	//mount sd card

	if (res != FR_OK) 		    // return if mount failed
        return;

    //---------------------------------------------- generate first line of csv file
    char str[lineSize];

    sprintf(str,"Timestamp [ms];");                 //timestamp at first column

    k_mutex_lock(&sensorBufferMutex,K_FOREVER);		//lock sensor buffer mutex

    for(int i=0;i<configFile.sensorCount;i++)       //print name of all sensors
    {
        sprintf(str,"%s%s;",str,sensorBuffer[i].name_log);
    }

    k_mutex_unlock(&sensorBufferMutex);		        //unlock sensor buffer mutex
/*
    k_mutex_lock(&gpsBufferMutex,K_FOREVER);		//lock gps buffer mutex

    sprintf(str,"%s%s;",str,gpsBuffer.NameLogCoord);        //print gps names
    sprintf(str,"%s%s;",str,gpsBuffer.NameLogSpeed);
    sprintf(str,"%s%s;",str,gpsBuffer.NameLogFix);
    
    k_mutex_unlock(&gpsBufferMutex);		        //unlock gps buffer mutex
*/
    sprintf(str,"%s\n",str);                        //append \n at end of line

    //---------------------------------------------------- generate filename
    
    uint16_t logNumber = 0;
    
    //read flash
    int rc = nvs_read(&fs, LOGNAME_ID, &logNumber, sizeof(logNumber));

	if (rc > 0)                      
        logNumber++;             // if item was found increment number 
	

    //add new log Number in the flash memory
    (void)nvs_write(&fs, LOGNAME_ID, &logNumber, sizeof(logNumber));


    char fileName[15];                   //file name

    sprintf(fileName,"LOG_%04d",logNumber);    //generate file name with file number

    //---------------------------------------------------- Create file and write first line

    fs_file_t_init(&logFile);                   //init file object

    char path[25];
    sprintf(path,"/SD:/%s.csv",fileName);       //generate file path

    //create and open file on SD card
    res = fs_open(&logFile,path,FS_O_CREATE | FS_O_WRITE);
    if (res != 0) 		                // return if file creation failed
        return;

    //write in file
    res = fs_write(&logFile,str,strlen(str));
    if (res < 0) 		     // return if write failed
        return;

    //set log enable to true
    logEnable=true;

    //set timestamp
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

    //set recording status on the can
    (*recordOFF)();
}


//-----------------------------------------------------------------------------------------------------------------------
/*! Task_Data_Logger_Init initializes the task Data_Logger
*
* @brief Data Logger initialization
*/
void Task_Data_Logger_Init(void)
{
    /* define the nvs file system by settings with:
	 *	sector_size equal to the pagesize,
	 *	starting at NVS_PARTITION_OFFSET
	 */
    struct flash_pages_info info;
	fs.flash_device = NVS_PARTITION_DEVICE;
	if (!device_is_ready(fs.flash_device)) {
		LOG_ERR("Flash device %s is not ready\n", fs.flash_device->name);
		return;
	}
	fs.offset = NVS_PARTITION_OFFSET;
	int rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
	if (rc) {
		LOG_ERR("Unable to get page info\n");
		return;
	}
	fs.sector_size = info.size;
	fs.sector_count = 3U;

	rc = nvs_mount(&fs);
	if (rc) {
		printk("Flash Init failed\n");
		return;
	}

    //set log recording variable to false
	logEnable=false;

    //calculate line size
    lineSize=15;          //size for "Timestamp [ms];"
    for(int i=0;i<configFile.sensorCount;i++)
    {
        if(strlen(sensorBuffer[i].name_log)<10)                 //if name is shorter than 10
            lineSize+=11;                                       // add max length of 32 bit variable in decimal + 1 for the ;
        else                                                    //else
            lineSize+=(1+strlen(sensorBuffer[i].name_log));     // add string length of name + 1 for the ;
    }

/*
    //add size of gps coordinates
    if(strlen(gpsBuffer.NameLiveCoord)<25)                  //if name is shorter than 25
        lineSize+=26;                                       // add max length of gps coordinates + 1 for the ;
    else                                                    //else
        lineSize+=(1+strlen(gpsBuffer.NameLiveCoord));      // add string length of name + 1 for the ;

    //add size of gps speed
    if(strlen(gpsBuffer.NameLiveSpeed)<7)                   //if name is shorter than 7
        lineSize+=8;                                        // add max length of gps speed + 1 for the ;
    else                                                    //else
        lineSize+=(1+strlen(gpsBuffer.NameLiveSpeed));      // add string length of name + 1 for the ;

    //add size of gps fix
    if(strlen(gpsBuffer.NameLiveFix)<5)                     //if name is shorter than 5
        lineSize+=6;                                        //  add max length of gps speed + 1 for the ;
    else                                                    //else
        lineSize+=(1+strlen(gpsBuffer.NameLiveFix));        // add string length of name + 1 for the ;
*/
    //start timer
    k_timer_start(&dataLoggerTimer, K_SECONDS(0), K_MSEC((int)(1000/configFile.LogFrameRate)));

    
}