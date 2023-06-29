#include <zephyr/kernel.h>

#define JSON_TRANSMIT_SIZE 400

//memory heap
extern struct k_heap memHeap;

//queues
extern struct k_queue udpQueue;