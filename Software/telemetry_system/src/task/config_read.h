#ifndef __CONFIG_READ_H
#define __CONFIG_READ_H

#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>
#include "memory_management.h"


void read_config(void);




struct sWiFiRouter{
    char* SSID;
    char* Password;
};
struct sWiFiRouterRedundancy{
    char* SSID;
    char* Password;
    bool Enabled;
};
struct sServer{
    char* address;
    int port;
};
struct sSensors{
    char* name;
    bool live;
};

struct config {
	struct sWiFiRouter WiFiRouter;
    struct sWiFiRouterRedundancy WiFiRouterRedundancy;
    struct sServer Server[MAX_SERVERS];
    int TelemetyDataSize;
	int serverNumber;
    struct sSensors Sensors[MAX_SENSORS];
    int sensorNumber;
};

//config file datas accessible in other files
extern struct config configFile;




#endif /*__CONFIG_READ_H*/