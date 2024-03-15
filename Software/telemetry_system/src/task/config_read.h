/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file config_read.h
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

#ifndef __CONFIG_READ_H
#define __CONFIG_READ_H

#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>
#include "memory_management.h"


/*! @brief read_config reads the config file and put the datas in a config struct	
* @retval 0 on success
* @retval 1 on json error
* @retval 2 on disk error
* @retval 3 on file error
*/
int read_config(void);


/*! @brief struct for the Wi-Fi router configuration
* @param SSID SSID of the Wi-Fi router
* @param Password Password of the Wi-Fi router
*/
struct sWiFiRouter{
    const char* SSID;
    char* Password;
};

/*! @brief struct for the redundancy Wi-Fi router configuration
* @param SSID SSID of the Wi-Fi router
* @param Password Password of the Wi-Fi router
* @param Enabled boolean variable to activate the redundancy
*/
struct sWiFiRouterRedundancy{
    const char* SSID;
    char* Password;
    bool Enabled;
};

/*! @brief struct for the Server configuration
* @param address IP address of Server
* @param port port of server
*/
struct sServer{
    char* address;
    int port;
};

/*! @brief struct for the can filter configuration
* @param id id to match
* @param mask mask
*/
struct sCANFilter{
    char* id;
    char* mask;
};

/*! @brief struct for the can button configuration
* @param CanID CAN ID of the message
* @param index index of the byte in the message
* @param match byte to match
* @param mask mask on the byte
*/
struct sCANButtonData{
    char* CanID;
    int index;
    char* match;
    char* mask;
    int dlc;
};

/*! @brief struct for the can button configuration
* @param StartLog start button params
* @param StopLog stop button params
*/
struct sCANButton{
    struct sCANButtonData StartLog;
    struct sCANButtonData StopLog;
};

/*! @brief struct for the can led configuration
* @param CanID CAN ID of the message
*/
struct sCANLed{
    char* CanID; 
};
    
/*! @brief struct for the GPS datapoint
* @param NameLive Name of the datapoint on the live data transmission
* @param NameLog Name of the datapoint on the logs
* @param LiveEnable Enable datapoint on the live data transmisson
*/
struct sGPSData{
    char* NameLive;
    char* NameLog;
    bool LiveEnable;
};

/*! @brief struct for the GPS data
* @param Coordinates GPS Coordinate struct
* @param Speed GPS speed struct
* @param Fix GPS fix struct
*/
struct sGPS{
    struct sGPSData Coordinates;
    struct sGPSData Speed;
    struct sGPSData Fix;
};

/*! @brief struct for the CAN datapoint
* @param NameLive Name of the datapoint on the live data transmission
* @param NameLog Name of the datapoint on the logs
* @param LiveEnable Enable datapoint on the live data transmisson
* @param CanID CAN id of the message containing the datapoint value
* @param CanFram Config frame of the can message containing the datapoint value
*/
struct sSensors{
    char* NameLive;
    char* NameLog;
    bool LiveEnable;
    char * CanID;
    char * CanFrame;
};

/*! @brief main config struct
* @param WiFiRouter WiFi router struct
* @param WiFiRouterRedundancy Redundancy WiFi router struct
* @param Server Server struct array
* @param serverCount number of servers
* @param LiveFrameRate Live send frequency (sends/second)
* @param LogFrameRate Log record frequency (records/second)
* @param CANFilter Can filter
* @param GPS GPS config struct
* @param Sensors Sensor struct array
* @param sensorCount number of sensors
*/
struct config {
	struct sWiFiRouter WiFiRouter;
    struct sWiFiRouterRedundancy WiFiRouterRedundancy;
    struct sServer Server[MAX_SERVERS];
	int serverCount;
    int LiveFrameRate;
    int LogFrameRate;
    struct sCANFilter CANFilter;
    struct sCANButton CANButton;
    struct sCANLed CANLed;
    struct sGPS GPS;
    struct sSensors Sensors[MAX_SENSORS];
    int sensorCount;
};

//config file datas accessible in other files
extern struct config configFile;

//config ok
extern bool configOK;

#endif /*__CONFIG_READ_H*/