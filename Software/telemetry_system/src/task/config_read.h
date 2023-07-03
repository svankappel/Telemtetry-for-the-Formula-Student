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
    char* SSID;
    char* Password;
};

//struct for redundancy Wifi router data
struct sWiFiRouterRedundancy{
    char* SSID;
    char* Password;
    bool Enabled;
};

//struct for udp servers
struct sServer{
    char* address;
    int port;
};

//struct for sensors
struct sSensors{
    char* name;
    bool live;
};

//main config struct containing all the previous ones
struct config {
	struct sWiFiRouter WiFiRouter;
    struct sWiFiRouterRedundancy WiFiRouterRedundancy;
    struct sServer Server[MAX_SERVERS];
	int serverNumber;
    struct sSensors Sensors[MAX_SENSORS];
    int sensorNumber;
};

//config file datas accessible in other files
extern struct config configFile;

//config ok
extern bool configOK;

#endif /*__CONFIG_READ_H*/