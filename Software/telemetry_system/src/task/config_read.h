#ifndef __CONFIG_READ_H
#define __CONFIG_READ_H

#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>
#include "memory_management.h"


/*! read_config function
* @brief read_config reads the config file and put the datas in a struct	
* return 0 when config file is ok
*/
int read_config(void);

//struct for Wifi router data
struct sWiFiRouter{
    const char* SSID;
    const char* Password;
};

//struct for redundancy Wifi router data
struct sWiFiRouterRedundancy{
    const char* SSID;
    const char* Password;
    bool Enabled;
};

//struct for udp servers
struct sServer{
    const char* address;
    int port;
};
    
//struct for gps data
struct sGPSData{
    const char* NameLive;
    const char* NameLog;
    bool LiveEnable;
};

//struct for GPS
struct sGPS{
    struct sGPSData Coordinates;
    struct sGPSData Speed;
    struct sGPSData Fix;
};

//struct for Can sensors
struct sSensors{
    const char* NameLive;
    const char* NameLog;
    bool LiveEnable;
    const char * CanID;
    const char * CanFrame;
};

//main config struct containing all the previous ones
struct config {
	struct sWiFiRouter WiFiRouter;
    struct sWiFiRouterRedundancy WiFiRouterRedundancy;
    struct sServer Server[MAX_SERVERS];
	int serverCount;
    int LiveFrameRate;
    int LogFrameRate;
    struct sGPS GPS;
    struct sSensors Sensors[MAX_SENSORS];
    int sensorCount;
};

//config file datas accessible in other files
extern struct config configFile;

//config ok
extern bool configOK;

#endif /*__CONFIG_READ_H*/