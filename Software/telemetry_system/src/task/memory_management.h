#ifndef __MEMORY_MANAGEMENT_H
#define __MEMORY_MANAGEMENT_H

#include <zephyr/kernel.h>

#define MAX_SERVERS 5
#define MAX_SENSORS 100

//memory heap
extern struct k_heap messageHeap;

//queues
extern struct k_queue udpQueue;
extern int udpQueueMesLength;

//sensor buffer
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

//gps buffer
typedef struct sGps{
    char speed[10];
    char coord[25];
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

//extern global variables for sensor buffer and gps buffer
extern tSensor sensorBuffer[MAX_SENSORS];
extern struct k_mutex sensorBufferMutex;

extern tGps gpsBuffer;
extern struct k_mutex gpsBufferMutex;

#endif /*__MEMORY_MANAGEMENT_H*/