#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(sta, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>
#include <zephyr/data/json.h>
#include <zephyr/random/rand32.h>

#include "data_sender.h"
#include "memory_management.h"
#include "deviceInformation.h"
#include "config_read.h"

//! Stack size for the data sender thread
#define DATA_SENDER_STACK_SIZE 2048
//! data sender thread priority level
#define DATA_SENDER_PRIORITY 5

//! data sender stack definition
K_THREAD_STACK_DEFINE(DATA_SENDER_STACK, DATA_SENDER_STACK_SIZE);

//! Variable to identify the data sender thread
static struct k_thread dataSenderThread;
	
int udpQueueMesLength;

void Data_Sender() 
{
	uint8_t keepAliveCounter = 0;

	udpQueueMesLength = 0;
	for(int i=0; i<configFile.sensorNumber;i++)
	{
		if(sensorBuffer[i].live)
			udpQueueMesLength+=(strlen(sensorBuffer[i].name)+4+10);	//name length + 4 bytes for ,:"" + 10 bytes for number (32bits in decimal)
	}
	udpQueueMesLength+=50;		// space for {} , logRecording and keepalive

	while(true)
	{
		for(int i=0; i<configFile.sensorNumber;i++)
		{
			sensorBuffer[i].value=sys_rand32_get()%100;
		}

		if(context.ip_assigned)
		{
			char * memPtr = k_heap_alloc(&messageHeap,udpQueueMesLength,K_NO_WAIT);

			if(memPtr != NULL)			//memory alloc success
			{
				sprintf(memPtr,"{");
				bool first = true;
				for(int i=0; i<configFile.sensorNumber;i++)
				{
					if(sensorBuffer[i].live)
					{
						if(first)
							sprintf(memPtr,"%s\"%s\":%d",memPtr,sensorBuffer[i].name,sensorBuffer[i].value);
						else
							sprintf(memPtr,"%s,\"%s\":%d",memPtr,sensorBuffer[i].name,sensorBuffer[i].value);
						first=false;
					}
				}
				sprintf(memPtr,"%s,\"KeepAliveCounter\":%d",memPtr,keepAliveCounter);
				true ? sprintf(memPtr,"%s,\"LogRecordingSD\":true",memPtr) : sprintf(memPtr,"%s,\"LogRecordingSD\":false",memPtr) ;
				strcat(memPtr,"}");

				k_queue_append(&udpQueue,memPtr);
			}
			else //mem alloc fail
			{
				LOG_ERR("data sender memory allocation failed");
			}
		}

		keepAliveCounter = keepAliveCounter<99 ? keepAliveCounter+1 : 0 ;
		k_msleep(500);
	}
	
}

/*! Task_UDP_Client_Init initializes the task UDP Client
*
* @brief UDP Client initialization
*/
void Task_Data_Sender_Init( void ){
	k_thread_create	(														\
					&dataSenderThread,										\
					DATA_SENDER_STACK,										\
					DATA_SENDER_STACK_SIZE,									\
					(k_thread_entry_t)Data_Sender,							\
					NULL,													\
					NULL,													\
					NULL,													\
					DATA_SENDER_PRIORITY,									\
					0,														\
					K_NO_WAIT);	

	 k_thread_name_set(&dataSenderThread, "dataSender");
	 k_thread_start(&dataSenderThread);
}
