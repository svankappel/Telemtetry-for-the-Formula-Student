/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file config_read.c
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief reads the configuration file on the SD card
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
LOG_MODULE_REGISTER(config, 4);
#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/data/json.h>
#include <zephyr/fs/fs.h>
#include <ff.h>
#include <stdlib.h>
#include <zephyr/toolchain.h>
#include <string.h>

#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/uart.h>

//include project files
#include "config_read.h"
#include "memory_management.h"

//file system
static FATFS fat_fs;

// SD mounting info 
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};

// SD mount name
static const char *disk_mount_pt = "/SD:";

//json config file struct
struct config configFile;
bool configOK;		//boolean variable for printing state on led
const int totalSends = 50;

static bool uart_tx_completed = true;
static bool uart_rx_ready = false;
static uint8_t sd_card_buffer[512];


// uart node
#define UART_DEVICE_NODE_CONFIG DT_CHOSEN(zephyr_shell_uart_config)
static const struct device *const uart_dev_config = DEVICE_DT_GET(UART_DEVICE_NODE_CONFIG);
void serial_cb_config(const struct device *dev, void *user_data);
bool sendFinished = false;

//struct for Wifi router data description
static const struct json_obj_descr wifi_router_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct sWiFiRouter, SSID, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sWiFiRouter, Password, JSON_TOK_STRING)
};

//struct for redundancy Wifi router data description
static const struct json_obj_descr wifi_router_red_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct sWiFiRouterRedundancy, SSID, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sWiFiRouterRedundancy, Password, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sWiFiRouterRedundancy, Enabled, JSON_TOK_TRUE)
};

//struct for udp servers description
static const struct json_obj_descr server_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct sServer, address, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sServer, port, JSON_TOK_NUMBER)
};

//struct for GPS data description
static const struct json_obj_descr gpsdata_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct sGPSData, NameLive, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sGPSData, NameLog, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sGPSData, LiveEnable, JSON_TOK_TRUE),
};

//struct for CAN filter description
static const struct json_obj_descr canfilter_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct sCANFilter, id, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sCANFilter, mask, JSON_TOK_STRING)
};

//struct for CAN button data description
static const struct json_obj_descr canbuttondata_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct sCANButtonData, CanID, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sCANButtonData, index, JSON_TOK_NUMBER),
	JSON_OBJ_DESCR_PRIM(struct sCANButtonData, match, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sCANButtonData, mask, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sCANButtonData, dlc, JSON_TOK_NUMBER)
};

//struct for CAN button description
static const struct json_obj_descr canbutton_descr[] = {
	JSON_OBJ_DESCR_OBJECT(struct sCANButton, StartLog, canbuttondata_descr),
  	JSON_OBJ_DESCR_OBJECT(struct sCANButton, StopLog, canbuttondata_descr)
};

//struct for CAN Led description
static const struct json_obj_descr canled_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct sCANLed, CanID, JSON_TOK_STRING)
};

//struct for GPS description
static const struct json_obj_descr gps_descr[] = {
  JSON_OBJ_DESCR_OBJECT(struct sGPS, Coordinates, gpsdata_descr),
  JSON_OBJ_DESCR_OBJECT(struct sGPS, Speed, gpsdata_descr),
  JSON_OBJ_DESCR_OBJECT(struct sGPS, Fix, gpsdata_descr)
};

//struct for sensors description
static const struct json_obj_descr sensors_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct sSensors, NameLive, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sSensors, NameLog, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sSensors, LiveEnable, JSON_TOK_TRUE),
	JSON_OBJ_DESCR_PRIM(struct sSensors, CanID, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sSensors, CanFrame, JSON_TOK_STRING),
};

//main config struct description
static const struct json_obj_descr config_descr[] = {
  JSON_OBJ_DESCR_OBJECT(struct config, WiFiRouter, wifi_router_descr),
  JSON_OBJ_DESCR_OBJECT(struct config, WiFiRouterRedundancy, wifi_router_red_descr),
  JSON_OBJ_DESCR_OBJ_ARRAY(struct config, Server, MAX_SERVERS, serverCount, server_descr,ARRAY_SIZE(server_descr)),
  JSON_OBJ_DESCR_PRIM(struct config, LiveFrameRate, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct config, LogFrameRate, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct config, RecordOnStart, JSON_TOK_TRUE),
  JSON_OBJ_DESCR_OBJECT(struct config, CANFilter, canfilter_descr),
  JSON_OBJ_DESCR_OBJECT(struct config, CANButton, canbutton_descr),
  JSON_OBJ_DESCR_OBJECT(struct config, CANLed, canled_descr),
  JSON_OBJ_DESCR_OBJECT(struct config, GPS, gps_descr),
  JSON_OBJ_DESCR_OBJ_ARRAY(struct config, Sensors, MAX_SENSORS, sensorCount, sensors_descr,ARRAY_SIZE(sensors_descr))
};



//-----------------------------------------------------------------------------------------------------------------------
/*! @brief read_config reads the config file and put the datas in a config struct	
* @retval 0 on success
* @retval 1 on json error
* @retval 2 on disk error
* @retval 3 on file error
* @retval 4 on uart error
*/
int read_config(void)
{

	//------------------------------------------------  Mount SD disk
	mp.mnt_point = disk_mount_pt;

	int res = fs_mount(&mp);	//mount sd card

	if (res == FR_OK) 		//return error if mount failed
	{
		LOG_INF("Disk mounted.\n");
	} 
	else 
	{
		LOG_ERR("Error mounting disk.\n");
		return 2;
	}


	// -----------------------------------------------      Find config file

	struct fs_dir_t dirp;
	static struct fs_dirent entry;

	fs_dir_t_init(&dirp);					//initialize directory

	// Verify fs_opendir() 			
	res = fs_opendir(&dirp, "/SD:");		//open SD card base directory
	if (res) 								//return error if open failed
	{
		LOG_ERR("Error opening dir /SD: [%d]\n" , res);
		return 2;
	}

	for (;;) 								//loop to find config file
	{
		res = fs_readdir(&dirp, &entry);	//read directory

		if (res || entry.name[0] == 0) 		//no more files
			return 3;						//return error code

		if (entry.type == FS_DIR_ENTRY_FILE)				//file found
		{
			LOG_INF("[FILE] %s (size = %zu)\n",entry.name, entry.size);
			if(strstr(entry.name, "CONF") != NULL)			//check if file is config file
				break;										//exit loop
		} 
	}

	fs_closedir(&dirp);	//close directory


	// ------------------------------------------   Read file

	//create and init file
	struct fs_file_t fs_configFile;
	fs_file_t_init(&fs_configFile);

	//path with the name found in the previous section
	char path[20];	
	sprintf(path,"/SD:/");
	strcat(path,entry.name);

	//open file
	res = fs_open(&fs_configFile,path,FS_O_READ);	

	if (res) 			//return error if open fail
	{
		LOG_ERR("Error opening dir /SD: [%d]\n" , res);
		return 2;
	}

	entry.size*=2; //take some margin (according to some tests the size read could be too small)

	uint8_t readBuf[entry.size];					//buffer for the content

	for(int i = 0; i<=entry.size;i++)	//initialize buffer
		readBuf[i]=0;

	fs_read(&fs_configFile,readBuf,entry.size);		//read file


	fs_close(&fs_configFile);					//close file

	fs_unmount(&mp);							//unmount sd disk
	
	//--------------------------------------- parse json string

	//parse json
	int ret = json_obj_parse(readBuf,entry.size,config_descr,ARRAY_SIZE(config_descr),&configFile);
	
	if(ret<0)				//if json parse fail
	{
		LOG_ERR("Error reading config file");	//print error
		configOK=false;
		return 1;
	}
	else					//json parse success
	{		
		LOG_INF("Config file OK");				//print message		
		configOK=true;				
		
		//----------------------------------------------------------------------------------initialize sensor buffer

		for(int i=0; i<configFile.sensorCount;i++)	 //for all sensors
		{
			//configure sensor buffer
			sensorBuffer[i].name_wifi=configFile.Sensors[i].NameLive;			//set name on live
			sensorBuffer[i].name_log=configFile.Sensors[i].NameLog;				//set name on logs
			sensorBuffer[i].wifi_enable=configFile.Sensors[i].LiveEnable;		//sensor active on live
			sensorBuffer[i].value=0;											//initialize sensor value
			sensorBuffer[i].canID=(uint32_t)strtol(configFile.Sensors[i].CanID, NULL, 0);	//set can ID
			
			//initialize position of bytes in CAN frame from config file 
			// config file form : "CanFrame":"X:X:B2:B1:X:X:X:X"

			sensorBuffer[i].B1=-1;
			sensorBuffer[i].B2=-1;
			sensorBuffer[i].B3=-1;
			sensorBuffer[i].B4=-1;

			char * saveptr=NULL;									//strtok save pointer
			char * str = configFile.Sensors[i].CanFrame;			//string of frame
			char * token = (char*)strtok_r(str,":",&saveptr);		//get first token

			int idx = 0;		//loop index

			//loop for all token of config frame
			while (token!=NULL) 
			{
				if(strcmp(token,"X")==0)						// X -> no conditions
				{
					sensorBuffer[i].conditions[idx]=-1;
				}
				else if(strcmp(token,"B1")==0)					// B1 -> set B1 with current position
				{
					sensorBuffer[i].B1=idx;
					sensorBuffer[i].conditions[idx]=-1;
				}
				else if(strcmp(token,"B2")==0)					// B2 -> set B2 with current position
				{
					sensorBuffer[i].B2=idx;
					sensorBuffer[i].conditions[idx]=-1;
				}
				else if(strcmp(token,"B3")==0)					// B3 -> set B3 with current position
				{
					sensorBuffer[i].B3=idx;
					sensorBuffer[i].conditions[idx]=-1;
				}
				else if(strcmp(token,"B4")==0)					// B4 -> set B4 with current position
				{
					sensorBuffer[i].B4=idx;
					sensorBuffer[i].conditions[idx]=-1;
				}
				else											// else -> number -> set condition array
				{
					sensorBuffer[i].conditions[idx]=(int)strtol(token, NULL, 0);
				}

				
				token = (char*)strtok_r(NULL,":",&saveptr);				// get next token
				idx++;
			}
			sensorBuffer[i].dlc=idx;

		}

		//------------------------------------------------------------------------------------  initialize gps buffer
		/*
		//initialize gps buffer values
		gpsBuffer.fix=false;
		strcpy(gpsBuffer.speed,"0.0");
		strcpy(gpsBuffer.coord,"0.0 0.0");
		
		//initialize gpsbuffer parames with config file values
		gpsBuffer.LiveCoordEnable=configFile.GPS.Coordinates.LiveEnable;
		gpsBuffer.LiveSpeedEnable=configFile.GPS.Speed.LiveEnable;
		gpsBuffer.LiveFixEnable=configFile.GPS.Fix.LiveEnable;
		gpsBuffer.NameLiveCoord=configFile.GPS.Coordinates.NameLive;
		gpsBuffer.NameLogCoord=configFile.GPS.Coordinates.NameLog;
		gpsBuffer.NameLiveSpeed=configFile.GPS.Speed.NameLive;
		gpsBuffer.NameLogSpeed=configFile.GPS.Speed.NameLog;
		gpsBuffer.NameLiveFix=configFile.GPS.Fix.NameLive;
		gpsBuffer.NameLogFix=configFile.GPS.Fix.NameLog;

		*/

		//------------------------------------------------------------------------------------  send config to transmitter

		k_sleep(K_MSEC(1000));
		LOG_INF("Start to send config");

		//check if uart device is ready
		if (!device_is_ready(uart_dev_config)) 
		{
			LOG_INF("UART device not found!");
			return 4;
		}

		

		// configure interrupt and callback to receive data 
		memcpy(sd_card_buffer, readBuf, sizeof(sd_card_buffer));
		uart_irq_callback_user_data_set(uart_dev_config, serial_cb_config, NULL);
		//uart_irq_rx_enable(uart_dev_config);
		uart_irq_tx_enable(uart_dev_config);

		
		while(!sendFinished)
		{
			k_sleep(K_MSEC(100));
		}
		
		LOG_INF("Configuration file sent to transmitter ( bytes)");
	
		return 0;
	}
	
}

//-----------------------------------------------------------------------------------------------------------------------
/*!
 * @brief Read characters from UART
 */
static int sent = 0;
static const int size = 512;
static int ret;

void serial_cb_config(const struct device *dev, void *user_data)
{
	if (!uart_irq_update(uart_dev_config)) {
		return;
	}

	//receiving data
	if (uart_irq_rx_ready(dev))
	{
		uart_rx_ready = true;
	}

	// Sending data
	if (uart_irq_tx_ready(dev))
	{
		if (sent < sizeof(sd_card_buffer))
		{
			ret = uart_fifo_fill(dev, &sd_card_buffer[sent],size);
			if (ret > 0 && ret <= size)
			{
				//LOG_INF("%d data sent over uart", ret);
				sent+=ret;
			}
		}
		else if (sent == sizeof(sd_card_buffer))
		{
			// copy next chunck

			//end character
			const uint8_t end = 0x14;
			uart_fifo_fill(dev, &end, 1);
			sent++;

			sendFinished = true;
			uart_irq_tx_disable(dev);	
		}
	}
}