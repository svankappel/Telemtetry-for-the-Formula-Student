#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>
#include <zephyr/data/json.h>
#include "config_read.h"


#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(sta, LOG_LEVEL_DBG);

uint8_t configString[] = 
"{\"WiFiRouter\":{\"SSID\":\"VRT-Telemetry\",\"Password\":\"TJJC2233\"},\"WiFiRouterRedundancy\":{\"SSID\":\"motog8\",\"Password\":\"TJJC2233\",\"Enabled\": false},"
"\"Server\":[{\"address\":\"192.168.50.110\",\"port\":1502}],\"TelemetyDataSize\":400,\"Sensors\":[{\"name\":\"TensionBatteryHV\",\"live\":true},{\"name\":\"AmperageBatteryHV\",\"live\":true},"
"{\"name\":\"TemperatureBatteryHV\",\"live\":true},{\"name\":\"EnginePower\",\"live\":true},{\"name\":\"EngineTemperature\",\"live\":true},"
"{\"name\":\"EngineAngularSpeed\",\"live\":true},{\"name\":\"CarSpeed\",\"live\":true},{\"name\":\"PressureTireFL\",\"live\":true},"
"{\"name\":\"PressureTireFR\",\"live\":true},{\"name\":\"PressureTireBL\",\"live\":true},{\"name\":\"PressureTireBR\",\"live\":true},"
"{\"name\":\"InverterTemperature\",\"live\":true},{\"name\":\"TemperatureBatteryLV\",\"live\":true}]}";

struct config configFile;


static const struct json_obj_descr wifi_router_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct sWiFiRouter, SSID, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sWiFiRouter, Password, JSON_TOK_STRING)
};

static const struct json_obj_descr wifi_router_red_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct sWiFiRouterRedundancy, SSID, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sWiFiRouterRedundancy, Password, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sWiFiRouterRedundancy, Enabled, JSON_TOK_TRUE)
};

static const struct json_obj_descr server_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct sServer, address, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sServer, port, JSON_TOK_NUMBER)
};

static const struct json_obj_descr sensors_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct sSensors, name, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct sSensors, live, JSON_TOK_TRUE)
};

static const struct json_obj_descr config_descr[] = {
  JSON_OBJ_DESCR_OBJECT(struct config, WiFiRouter, wifi_router_descr),
  JSON_OBJ_DESCR_OBJECT(struct config, WiFiRouterRedundancy, wifi_router_red_descr),
  JSON_OBJ_DESCR_PRIM(struct config, TelemetyDataSize, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_OBJ_ARRAY(struct config, Server, MAX_SERVERS, serverNumber, server_descr,ARRAY_SIZE(server_descr)),
  JSON_OBJ_DESCR_OBJ_ARRAY(struct config, Sensors, MAX_SENSORS, sensorNumber, sensors_descr,ARRAY_SIZE(sensors_descr))
};


void read_config(void)
{
	int ret = json_obj_parse(configString,sizeof(configString),config_descr,ARRAY_SIZE(config_descr),&configFile);

	if(ret<0)
	{
		LOG_ERR("Error reading config file");
		k_sleep(K_FOREVER);
	}
	else
	{
		LOG_INF("Config file OK");

		for(int i=0; i<configFile.sensorNumber;i++)
		{
			sensorBuffer[i].name=configFile.Sensors[i].name;
			sensorBuffer[i].live=configFile.Sensors[i].live;
		}


		return;
	}
	
}