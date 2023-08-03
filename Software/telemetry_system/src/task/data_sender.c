/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file data_sender.c
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief Data sender task. This task periodically sends the data from
 * 		  the sensor buffer and gps buffer to the udp client task
 * ---------------------------------------------------------------------
 * Telemetry system for the Valais Wallis Racing Team.
 * This file contains code for the onboard device of the telemetry
 * system. The system receives the data from the sensors on the CAN bus 
 * and the data from the GPS on a UART port. An SD Card contains a 
 * configuration file with all the system parameters. The measurements 
 * are sent via Wi-Fi to a computer on the base station. The measurements 
 * are also saved in a CSV file on the SD card. 
 *--------------------------------------------------------------------*/

//includes
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sender);
#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>
#include <zephyr/data/json.h>

//project file includes
#include "data_sender.h"
#include "memory_management.h"
#include "config_read.h"
#include "data_logger.h"
#include "deviceInformation.h"

//periodic timer that reads the measurements 
K_TIMER_DEFINE(dataSenderTimer, data_Sender_timer_handler,NULL);

//work for process triggerd by timer interruption
K_WORK_DEFINE(dataSendWork, Data_Sender);		//dataSendWork -> called by timer to send data

int udpQueueMesLength;		//max length of the json string
uint8_t keepAliveCounter;	//keepalive counter

//-----------------------------------------------------------------------------------------------------------------------
/*! data_Sender_timer_handler is called by the timer interrupt
* @brief data_Sender_timer_handler submit a new work that call Data_Sender task     
*/
void data_Sender_timer_handler()
{
    k_work_submit(&dataSendWork);
}

//-----------------------------------------------------------------------------------------------------------------------
/*! Data_Sender implements the Data_Sender task
* @brief Data_Sender reads the data in the sensor buffer array and
*        creates the json string to send via Wi-Fi to the base 
*        station. This string is placed in the UDP_Client queue.
*/
void Data_Sender() 
{
	if(context.ip_assigned)
	{
		char * memPtr = k_heap_alloc(&messageHeap,udpQueueMesLength,K_NO_WAIT);		//memory allocation for message string

		if(memPtr != NULL)			//memory alloc success
		{
			sprintf(memPtr,"{");	// open json section

			k_mutex_lock(&sensorBufferMutex,K_FOREVER);		//lock sensor buffer mutex

			for(int i=0; i<configFile.sensorCount;i++)	//loop for every sensor
			{
				if(sensorBuffer[i].wifi_enable)
				{
					//print name and value in json string
					if(i==0)
						sprintf(memPtr,"%s\"%s\":%u",memPtr,sensorBuffer[i].name_wifi,sensorBuffer[i].value);		
					else
						sprintf(memPtr,"%s,\"%s\":%u",memPtr,sensorBuffer[i].name_wifi,sensorBuffer[i].value);
				}
			}

			k_mutex_unlock(&sensorBufferMutex);				//unlock sensor buffer mutex

			k_mutex_lock(&gpsBufferMutex,K_FOREVER);		//lock gps buffer mutex
			
			if(gpsBuffer.LiveCoordEnable)
				sprintf(memPtr,"%s,\"%s\":\"%s\"",memPtr,gpsBuffer.NameLiveCoord,gpsBuffer.coord);
			
			if(gpsBuffer.LiveSpeedEnable)
				sprintf(memPtr,"%s,\"%s\":%s",memPtr,gpsBuffer.NameLiveSpeed,gpsBuffer.speed);

			if(gpsBuffer.LiveFixEnable && gpsBuffer.fix)
				sprintf(memPtr,"%s,\"%s\":true",memPtr,gpsBuffer.NameLiveFix);

			if(gpsBuffer.LiveFixEnable && !gpsBuffer.fix)
				sprintf(memPtr,"%s,\"%s\":false",memPtr,gpsBuffer.NameLiveFix);

			k_mutex_unlock(&gpsBufferMutex);				//unlock gps buffer mutex


			sprintf(memPtr,"%s,\"KeepAliveCounter\":%d",memPtr,keepAliveCounter);		//print keepalive counter in json
			
			if(logEnable)													//print log recording variable in json
				sprintf(memPtr,"%s,\"LogRecordingSD\":true",memPtr);
			else
				sprintf(memPtr,"%s,\"LogRecordingSD\":false",memPtr) ;	

			strcat(memPtr,"}");		//close json section
			
			k_queue_append(&udpQueue,memPtr);		//add message to the queue
		}
		else					 //memory alloc fail
		{
			LOG_ERR("data sender memory allocation failed");	//print error
		}	
	}
	

	keepAliveCounter = keepAliveCounter<99 ? keepAliveCounter+1 : 0 ;		//increment keepalive
}



//-----------------------------------------------------------------------------------------------------------------------
/*! Task_Data_Sender_Init initializes the task Data_Sender
*
* @brief Data Sender initialization
*/
void Task_Data_Sender_Init( void )
{
	keepAliveCounter = 0;		//initialize keep alive
	udpQueueMesLength = 0;				//initialize message length

	//calculate max length of json message for memory allocation
	for(int i=0; i<configFile.sensorCount;i++)		//loop for every sensor
	{
		if(sensorBuffer[i].wifi_enable)			//if sensor is used in live telemetry
			udpQueueMesLength+=(strlen(sensorBuffer[i].name_wifi)+4+10);	//name length + 4 bytes for ,:"" + 10 bytes for number (32bits in decimal)
	}

	if(gpsBuffer.LiveCoordEnable)			//if gps coord is used in live telemetry
			udpQueueMesLength+=(strlen(gpsBuffer.NameLiveCoord)+6+25);	//name length + 6 bytes for ,:"""" + 25 bytes for data
	
	if(gpsBuffer.LiveSpeedEnable)			//if gps coord is used in live telemetry
			udpQueueMesLength+=(strlen(gpsBuffer.NameLiveSpeed)+4+7);	//name length + 4 bytes for ,:"" + 7 bytes for data

	if(gpsBuffer.LiveFixEnable)			//if gps coord is used in live telemetry
			udpQueueMesLength+=(strlen(gpsBuffer.NameLiveFix)+4+5);	//name length + 4 bytes for ,:"" + 5 bytes for data

	udpQueueMesLength+=50;		// space for {} , logRecording and keepalive

	
	k_timer_start(&dataSenderTimer, K_SECONDS(0), K_MSEC((int)(1000/configFile.LiveFrameRate)));
}
