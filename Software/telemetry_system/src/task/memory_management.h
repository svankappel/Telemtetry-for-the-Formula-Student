#ifndef __MEMORY_MANAGEMENT_H
#define __MEMORY_MANAGEMENT_H

#include <zephyr/kernel.h>

#define MAX_SERVERS 5
#define MAX_SENSORS 100

//memory heap
extern struct k_heap messageHeap;
extern struct k_heap sensorHeap;

//queues
extern struct k_queue udpQueue;

//sensor buffer
typedef struct sSensor{
    char* name;
    uint32_t value;
    bool live;
}tSensor;

extern tSensor sensorBuffer[MAX_SENSORS];


#endif /*__MEMORY_MANAGEMENT_H*/