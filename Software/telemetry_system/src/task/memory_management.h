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
}tSensor;

extern tSensor sensorBuffer[MAX_SENSORS];
extern struct k_mutex sensorBufferMutex;

#endif /*__MEMORY_MANAGEMENT_H*/