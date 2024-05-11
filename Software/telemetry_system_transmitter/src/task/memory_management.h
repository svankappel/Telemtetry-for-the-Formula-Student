/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file memory_management.h
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief memory management file contains the extern declarations of 
 *        the variables, arrays, struct and queues that needs to be
 *        accessed in multiple files
 * ---------------------------------------------------------------------
 * Telemetry system for the Valais Wallis Racing Team.
 * This file contains code for the onboard device of the telemetry
 * system. The system receives the data from the sensors on the CAN bus 
 * and the data from the GPS on a UART port. An SD Card contains a 
 * configuration file with all the system parameters. The measurements 
 * are sent via Wi-Fi to a computer on the base station. The measurements 
 * are also saved in a CSV file on the SD card. 
 *--------------------------------------------------------------------*/

#ifndef __MEMORY_MANAGEMENT_H
#define __MEMORY_MANAGEMENT_H

#include <zephyr/kernel.h>

#define MAX_SERVERS 5           //max number of server the system can send data to
#define MAX_SENSORS 100         //max number of sensors

//memory heap for udp messages
extern struct k_heap messageHeap;


//queues
extern struct k_queue udpQueue;
extern int udpQueueMesLength;

/*! @brief sensor buffer struct
    @param name_wifi name of the sensor in the live transmission
    @param name_log name of the sensor in the logs
    @param value current value of the sensor
    @param wifi_enabled sensor value transmitted in the live
    @param canID CAN id of the message containing the value
    @param B1 position of 1st byte (LSB) in the CAN message
    @param B2 position of 2nd byte in the CAN message
    @param B3 position of 3rd byte in the CAN message
    @param B4 position of 4th byte (MSB) in the CAN message
    @param dlc length of the CAN message
    @param conditions conditions of the can message
*/
typedef struct sSensor{
    char* name_wifi;
    char* name_log;
    uint32_t value;
    bool wifi_enable;
    uint32_t canID;
    
    int B1;
    int B2;
    int B3;
    int B4;

    int dlc;

    int conditions[8];
}tSensor;
extern tSensor sensorBuffer[MAX_SENSORS];
extern struct k_mutex sensorBufferMutex;

extern bool logEnable;

/*! @brief gps buffer struct
    @param speed current gps speed
    @param coord current gps coords
    @param lat_sign sign of the latitude (0==- ; 1==+)
    @param lat_characteristic integer part of the latitude
    @param lat_mantissa fractionnal part of the latitude
    @param long_sign sign of the longitude (0==- ; 1==+)
    @param long_characteristic integer part of the longitude
    @param long_mantissa fractionnal part of the longitude
    @param fix current gps fix status
    @param NameLiveCoord name of the coord field in the live transmission
    @param NameLogCoord name of the coord field in the logs
    @param LiveCoordEnable coords enabled in the live transmission
    @param NameLiveSpeed name of the speed field in the live transmission
    @param NameLogSpeed name of the speed field in the logs
    @param LiveSpeedEnable speed enabled in the live transmission
    @param NameLiveFix name of the fix status field in the live transmission
    @param NameLogFix name of the fix status field in the logs
    @param LiveFixEnable fix status enabled in the live transmission
*/
typedef struct sGps{
    char speed[10];
    char coord[25];
    uint16_t lat_sign;
    uint16_t lat_characteristic;
    uint32_t lat_mantissa;
    uint16_t long_sign;
    uint16_t long_characteristic;
    uint32_t long_mantissa;
    bool fix;
    char * NameLiveCoord;
    char * NameLogCoord;
    bool LiveCoordEnable;
    char * NameLiveSpeed;
    char * NameLogSpeed;
    bool LiveSpeedEnable;
    char * NameLiveFix;
    char * NameLogFix;
    bool LiveFixEnable;
}tGps;
extern tGps gpsBuffer;
extern struct k_mutex gpsBufferMutex;

#endif /*__MEMORY_MANAGEMENT_H*/