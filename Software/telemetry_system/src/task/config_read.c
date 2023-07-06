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

static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};

/*
*  Note the fatfs library is able to mount only strings inside _VOLUME_STRS
*  in ffconf.h
*/
static const char *disk_mount_pt = "/SD:";

uint8_t configString[] = 
"{\"WiFiRouter\":{\"SSID\":\"VRT-Telemetry\",\"Password\":\"TJJC2233\"},\"WiFiRouterRedundancy\":{\"SSID\":\"motog8\",\"Password\":\"TJJC2233\",\"Enabled\": false},"
"\"Server\":[{\"address\":\"192.168.50.110\",\"port\":1502}],\"Sensors\":[{\"name\":\"TensionBatteryHV\",\"live\":true},{\"name\":\"AmperageBatteryHV\",\"live\":true},"
"{\"name\":\"TemperatureBatteryHV\",\"live\":true},{\"name\":\"EnginePower\",\"live\":true},{\"name\":\"EngineTemperature\",\"live\":true},"
"{\"name\":\"EngineAngularSpeed\",\"live\":true},{\"name\":\"CarSpeed\",\"live\":true},{\"name\":\"PressureTireFL\",\"live\":true},"
"{\"name\":\"PressureTireFR\",\"live\":true},{\"name\":\"PressureTireBL\",\"live\":true},{\"name\":\"PressureTireBR\",\"live\":true},"
"{\"name\":\"InverterTemperature\",\"live\":true},{\"name\":\"TemperatureBatteryLV\",\"live\":true}]}";

struct config configFile;
bool configOK;

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

//struct for sensors description
static const struct json_obj_descr sensors_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct sSensors, name, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sSensors, live, JSON_TOK_TRUE)
};

//main config struct description
static const struct json_obj_descr config_descr[] = {
  JSON_OBJ_DESCR_OBJECT(struct config, WiFiRouter, wifi_router_descr),
  JSON_OBJ_DESCR_OBJECT(struct config, WiFiRouterRedundancy, wifi_router_red_descr),
  JSON_OBJ_DESCR_OBJ_ARRAY(struct config, Server, MAX_SERVERS, serverNumber, server_descr,ARRAY_SIZE(server_descr)),
  JSON_OBJ_DESCR_OBJ_ARRAY(struct config, Sensors, MAX_SENSORS, sensorNumber, sensors_descr,ARRAY_SIZE(sensors_descr))
};

//-----------------------------------------------------------------------------------------------------------------------
/*! read_config function
* @brief read_config reads the config file and put the datas in a struct	
* return 0 when config file is ok, 1 in case of json error, 2 in case of sd card error, 3 if config file is not found
*/
int read_config(void)
{
	mp.mnt_point = disk_mount_pt;

	int res = fs_mount(&mp);

	if (res == FR_OK) 
	{
		LOG_INF("Disk mounted.\n");
	} 
	else 
	{
		LOG_ERR("Error mounting disk.\n");
		return 2;
	}

	struct fs_dir_t dirp;
	static struct fs_dirent entry;

	fs_dir_t_init(&dirp);

	/* Verify fs_opendir() */
	res = fs_opendir(&dirp, "/SD:");
	if (res) {
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


	struct fs_file_t myFile;
	fs_file_t_init(&myFile);

	char path[20];
	sprintf(path,"/SD:/");
	strcat(path,entry.name);

	res = fs_open(&myFile,path,FS_O_READ);

	if (res) {
		LOG_ERR("Error opening dir /SD: [%d]\n" , res);
		return 2;
	}
	
	char readBuf[entry.size];
	fs_read(&myFile,readBuf,entry.size);


	fs_close(&myFile);

	fs_unmount(&mp);

	//read json string
	int ret = json_obj_parse(readBuf,sizeof(readBuf),config_descr,ARRAY_SIZE(config_descr),&configFile);

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
		for(int i=0; i<configFile.sensorNumber;i++)	 //for all sensors
		{
			//configure sensor buffer
			sensorBuffer[i].name=configFile.Sensors[i].name;	
			sensorBuffer[i].live=configFile.Sensors[i].live;
		}
		return 0;
	}
	
}