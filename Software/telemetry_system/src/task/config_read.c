#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/data/json.h>
#include <zephyr/fs/fs.h>
#include <ff.h>

#include "config_read.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(config);

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
	JSON_OBJ_DESCR_PRIM(struct sGPSData, LiveEnable, JSON_TOK_TRUE)
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
	JSON_OBJ_DESCR_PRIM(struct sSensors, LiveEnable, JSON_TOK_TRUE)
};

//main config struct description
static const struct json_obj_descr config_descr[] = {
  JSON_OBJ_DESCR_OBJECT(struct config, WiFiRouter, wifi_router_descr),
  JSON_OBJ_DESCR_OBJECT(struct config, WiFiRouterRedundancy, wifi_router_red_descr),
  JSON_OBJ_DESCR_OBJ_ARRAY(struct config, Server, MAX_SERVERS, serverCount, server_descr,ARRAY_SIZE(server_descr)),
  JSON_OBJ_DESCR_PRIM(struct config, LiveFrameRate, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct config, LogFrameRate, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_OBJECT(struct config, GPS, gps_descr),
  JSON_OBJ_DESCR_OBJ_ARRAY(struct config, Sensors, MAX_SENSORS, sensorCount, sensors_descr,ARRAY_SIZE(sensors_descr))
};



//-----------------------------------------------------------------------------------------------------------------------
/*! read_config function
* @brief read_config reads the config file and put the datas in a struct	
* return 0 when config file is ok, 1 in case of json error, 2 in case of sd card error, 3 if config file is not found
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

	fs_dir_t_init(&dirp);		//initialize directory

	// Verify fs_opendir() 			
	res = fs_opendir(&dirp, "/SD:");		//open SD card base directory
	if (res) 								//return error if open failed
	{
		LOG_ERR("Error opening dir /SD: [%d]\n" , res);
		return 2;
	}

	for (;;) 			//loop to find config file
	{
		res = fs_readdir(&dirp, &entry);			//read directory

		if (res || entry.name[0] == 0) 		//no more files
			return 3;						//return error code

		if (entry.type == FS_DIR_ENTRY_FILE)		//file found
		{
			LOG_INF("[FILE] %s (size = %zu)\n",entry.name, entry.size);
			if(strstr(entry.name, "CONF") != NULL)				//check if file is config file
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
	
	uint8_t readBuf[entry.size];					//buffer for the content
	fs_read(&fs_configFile,readBuf,entry.size);		//read file


	fs_close(&fs_configFile);							//close file

	fs_unmount(&mp);							//unmount sd disk


	//--------------------------------------- parse json string

	int ret = json_obj_parse(readBuf,entry.size,config_descr,ARRAY_SIZE(config_descr),&configFile);
	
	if(ret<0)		//json parse fail
	{
		LOG_ERR("Error reading config file");	//print error
		configOK=false;
		return 1;
	}
	else			//json parse success
	{		
		LOG_INF("Config file OK");				//print message		
		configOK=true;
		
		//initialize sensor buffer
		for(int i=0; i<configFile.sensorCount;i++)	 //for all sensors
		{
			//configure sensor buffer
			sensorBuffer[i].name_wifi=configFile.Sensors[i].NameLive;	
			sensorBuffer[i].name_log=configFile.Sensors[i].NameLog;
			sensorBuffer[i].wifi_enable=configFile.Sensors[i].LiveEnable;
			sensorBuffer[i].value=0;
		}

		//initialize gps buffer
		gpsBuffer.fix=false;
		strcpy(gpsBuffer.speed,"0.0");
		strcpy(gpsBuffer.speed,"0.0 0.0");
		gpsBuffer.LiveCoordEnable=configFile.GPS.Coordinates.LiveEnable;
		gpsBuffer.LiveSpeedEnable=configFile.GPS.Speed.LiveEnable;
		gpsBuffer.LiveFixEnable=configFile.GPS.Fix.LiveEnable;
		gpsBuffer.NameLiveCoord=configFile.GPS.Coordinates.NameLive;
		gpsBuffer.NameLogCoord=configFile.GPS.Coordinates.NameLog;
		gpsBuffer.NameLiveSpeed=configFile.GPS.Speed.NameLive;
		gpsBuffer.NameLogSpeed=configFile.GPS.Speed.NameLog;
		gpsBuffer.NameLiveFix=configFile.GPS.Fix.NameLive;
		gpsBuffer.NameLogFix=configFile.GPS.Fix.NameLog;


		return 0;
	}
	
}