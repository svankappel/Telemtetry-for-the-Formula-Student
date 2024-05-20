/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file can_controller.c
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief Can Controller task receives data from the CAN Bus and fill 
 *        the sensor buffer with the received data
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
LOG_MODULE_REGISTER(can);
#include <nrfx_clock.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/can.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/byteorder.h>

//project files includes
#include "can_controller.h"
#include "memory_management.h"
#include "config_read.h"
#include "data_logger.h"

//! Can controller thread priority level
#define CAN_CONTROLLER_STACK_SIZE 8192
//! Can controller thread priority level
#define CAN_CONTROLLER_PRIORITY 4

//gps informations
tGps gpsBuffer;
K_MUTEX_DEFINE(gpsBufferMutex);

//! Can controller stack definition
K_THREAD_STACK_DEFINE(CAN_CONTROLLER_STACK, CAN_CONTROLLER_STACK_SIZE);
//! Variable to identify the can controller thread
static struct k_thread canControllerThread;

//sensor buffer and mutex to protect it

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
tSensor sensorBuffer[MAX_SENSORS];
K_MUTEX_DEFINE(sensorBufferMutex);

//can device
const struct device *const can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));

//can receive message queue
CAN_MSGQ_DEFINE(can_msgq, 100);

//can led
uint32_t canLedId;
uint32_t canLatId;
uint32_t canLongId;
uint32_t canTimeFixSpeedId;


typedef union{
	struct{
		uint16_t sign;
		uint16_t characteristic;
		uint32_t mantissa;
	}fields;
	uint8_t u8[8];
}Coord;

//-----------------------------------------------------------------------------------------------------------------------
/*! CAN_Controller implements the CAN_Controller task
* @brief CAN_Controller read the CAN Bus and fill the sensorBuffer array 
*        
*/
void CAN_Controller(void)
{
	//check if canbus device is ready	
	if (!device_is_ready(can_dev)) {
		LOG_ERR("CAN: Device %s not ready.\n", can_dev->name);
		return;
	}

	//start canbus controller
	int ret = can_start(can_dev);
	if (ret != 0) {
		LOG_ERR("Error starting CAN controller [%d]", ret);
		return;
	}

	//set can filter (mask = 0, takes all frames std and ext)
	const struct can_filter filter =
	{
		.flags=CAN_FILTER_DATA | CAN_FILTER_IDE,
		.id = (uint32_t)strtol(configFile.CANFilter.id, NULL, 0),
		.mask = (uint32_t)strtol(configFile.CANFilter.mask, NULL, 0)
	};
	can_add_rx_filter_msgq(can_dev, &can_msgq, &filter);

	//can button
	uint32_t canButtonId_start = (uint32_t)strtol(configFile.CANButton.StartLog.CanID, NULL, 0);
	uint8_t canButtonMatch_start = (uint8_t)strtol(configFile.CANButton.StartLog.match, NULL, 0);
	uint8_t canButtonMask_start = (uint8_t)strtol(configFile.CANButton.StartLog.mask, NULL, 0);
	uint8_t canButtonIndex_start = configFile.CANButton.StartLog.index;
	uint8_t canButtonDlc_start = configFile.CANButton.StartLog.dlc;

	uint32_t canButtonId_stop = (uint32_t)strtol(configFile.CANButton.StopLog.CanID, NULL, 0);
	uint8_t canButtonMatch_stop = (uint8_t)strtol(configFile.CANButton.StopLog.match, NULL, 0);
	uint8_t canButtonMask_stop = (uint8_t)strtol(configFile.CANButton.StopLog.mask, NULL, 0);
	uint8_t canButtonIndex_stop = configFile.CANButton.StopLog.index;
	uint8_t canButtonDlc_stop = configFile.CANButton.StopLog.dlc;

	//can ids
	canLedId = (uint32_t)strtol(configFile.CANLed.CanID, NULL, 0);
	canLatId = (uint32_t)strtol(configFile.GPS.CanIDs.Lat, NULL, 0);
	canLongId = (uint32_t)strtol(configFile.GPS.CanIDs.Long, NULL, 0);
	canTimeFixSpeedId = (uint32_t)strtol(configFile.GPS.CanIDs.TimeFixSpeed, NULL, 0); 
	
	//set recording callbacks
	set_RecordingStatus_callbacks(&recordingON,&recordingOFF);

	//variable to monitor the input buffer
	uint32_t bufferFill=0;
	uint32_t lastBufferFill=0;

	//frame struct
	struct can_frame frame;

	//purge input queue before starting the infinite loop
	k_msgq_purge(&can_msgq);

	while (1) 			//-------------------------------------------------- thread infinite loop
	{
		
		k_msgq_get(&can_msgq, &frame, K_FOREVER);		//get message from can message queue

		if((frame.id==canButtonId_start) && (frame.dlc == canButtonDlc_start))	//if we receive a message from can button canid
		{
			if(frame.data[canButtonIndex_start]==(canButtonMask_start & canButtonMatch_start))	//if can message at index
			{
				data_Logger_button_handler_start();		//call Data Logger button handler
			}
		}
		else if((frame.id==canButtonId_stop) && (frame.dlc == canButtonDlc_stop))	//if we receive a message from can button canid
		{
			if(frame.data[canButtonIndex_stop]==(canButtonMask_stop & canButtonMatch_stop))	//if can message at index
			{
				data_Logger_button_handler_stop();		//call Data Logger button handler
			}
		}
		else if((frame.id==canLatId) && (frame.dlc == 8))	//if we receive a message from gps - latitude
		{
			Coord latitude;
			memcpy(latitude.u8,frame.data,8);

			k_mutex_lock(&gpsBufferMutex,K_FOREVER);		    //lock gps buffer mutex

			gpsBuffer.lat_sign = latitude.fields.sign;
			gpsBuffer.lat_characteristic = latitude.fields.characteristic;
			gpsBuffer.lat_mantissa = latitude.fields.mantissa;
			
			k_mutex_unlock(&gpsBufferMutex);
		}
		else if((frame.id==canLongId) && (frame.dlc == 8))	//if we receive a message from gps - longitude
		{
			Coord longitude;
			memcpy(longitude.u8,frame.data,8);

			k_mutex_lock(&gpsBufferMutex,K_FOREVER);		    //lock gps buffer mutex

			gpsBuffer.long_sign = longitude.fields.sign;
			gpsBuffer.long_characteristic = longitude.fields.characteristic;
			gpsBuffer.long_mantissa = longitude.fields.mantissa;
			
			k_mutex_unlock(&gpsBufferMutex);
		}
		else if((frame.id==canTimeFixSpeedId) && (frame.dlc == 8))	//if we receive a message from gps - TimeFixSpeed
		{
			uint8_t data[8];
			memcpy(data,frame.data,8);

			k_mutex_lock(&gpsBufferMutex,K_FOREVER);		    //lock gps buffer mutex

			gpsBuffer.sec = data[7];
			gpsBuffer.min = data[6];
			gpsBuffer.hour = data[5];
			gpsBuffer.year = data[4];
			gpsBuffer.month = data[3];
			gpsBuffer.day = data[2];
			gpsBuffer.speed = data[1];
			gpsBuffer.fix = (data[0]==1);
			k_mutex_unlock(&gpsBufferMutex);
		}
		else
		{
			k_mutex_lock(&sensorBufferMutex,K_FOREVER);		//lock sensorBufferMutex
			for(int i = 0; i<configFile.sensorCount;i++)	//loop for all sensor of sensor buffer
			{
				if(sensorBuffer[i].canID==frame.id)			//if received can frame have the same id as the sensor
				{
					if(sensorBuffer[i].B1==-1 && sensorBuffer[i].B2==-1 &&		//if no bytes assigned (config file error)
						sensorBuffer[i].B1==-1 && sensorBuffer[i].B2==-1)
					{
						continue;												//continue loop
					}

					if(sensorBuffer[i].dlc != frame.dlc)						//continue lool if dlc error (config file error)
						continue;


					//check if the message has conditions
					bool conditionOk = true;
					for(int idx = 0; idx < frame.dlc; idx ++)
					{
						if(sensorBuffer[i].conditions[idx] != -1 && sensorBuffer[i].conditions[idx] != frame.data[idx])			//if conditions are not respected set variable to false
							conditionOk = false;
					}

					if(conditionOk)			//conditions ok
					{
						//get value in the message at the position of the B1,B2,B3,B4 variables
						sensorBuffer[i].value = 
						(uint32_t)frame.data[sensorBuffer[i].B1] + 
						(uint32_t)((sensorBuffer[i].B2 != -1) ? frame.data[sensorBuffer[i].B2] << 8 : 0) +
						(uint32_t)((sensorBuffer[i].B3 != -1) ? frame.data[sensorBuffer[i].B3] << 16 : 0) +
						(uint32_t)((sensorBuffer[i].B4 != -1) ? frame.data[sensorBuffer[i].B4] << 24 : 0) ;
					}
				}
			}
			k_mutex_unlock(&sensorBufferMutex);		//unlock mutex
		}
	
		//get fill of the buffer
		bufferFill=k_msgq_num_used_get(&can_msgq);	
		if(bufferFill >= 90)
		{
			LOG_ERR("CAN receive buffer overflowed !");			//print error if can buffer overflowed
			k_msgq_purge(&can_msgq);
		}
		else
		{
			if(bufferFill/10 != lastBufferFill)				//print warning if can buffer is filing too fast
			{
				lastBufferFill = bufferFill/10;

				LOG_WRN("CAN receive buffer between %d0 and %d0",bufferFill/10,bufferFill/10+1);
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------
/*! recording ON
* @brief set recording status on the can
*      
*/
void recordingON( void )
{
	struct can_frame frame = {
        .flags = 0,
        .id = canLedId,
        .dlc = 1,
        .data = {1}
	};

	int ret;

	ret = can_send(can_dev, &frame, K_MSEC(100), NULL, NULL);
	if (ret != 0) 
		LOG_ERR("Sending failed [%d]", ret);
}

//-----------------------------------------------------------------------------------------------------------------------
/*! recording OFF
* @brief set recording status on the can
*      
*/
void recordingOFF( void )
{
	struct can_frame frame = {
		.flags = 0,
		.id = canLedId,
		.dlc = 1,
		.data = {0}
	};

	int ret;

	ret = can_send(can_dev, &frame, K_MSEC(100), NULL, NULL);
	if (ret != 0) 
		LOG_ERR("Sending failed [%d]", ret);
}



//-----------------------------------------------------------------------------------------------------------------------
/*! Task_CAN_Controller_Init implements the CAN_Controller task initialization
* @brief CAN_Controller thread initialization
*      
*/
void Task_CAN_Controller_Init( void )
{
	k_thread_create	(&canControllerThread,
					CAN_CONTROLLER_STACK,										        
					CAN_CONTROLLER_STACK_SIZE,
					(k_thread_entry_t)CAN_Controller,
					NULL,
					NULL,
					NULL,
					CAN_CONTROLLER_PRIORITY,
					0,
					K_SECONDS(5));	

	 k_thread_name_set(&canControllerThread, "canController");
	 k_thread_start(&canControllerThread);
}



