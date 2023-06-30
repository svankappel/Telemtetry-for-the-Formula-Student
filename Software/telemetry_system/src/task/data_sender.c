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

struct data {
	uint32_t TensionBatteryHV;
	uint32_t AmperageBatteryHV;
	uint32_t TemperatureBatteryHV;
	uint32_t EnginePower;
	uint32_t EngineTemperature;
	uint32_t EngineAngularSpeed;
	uint32_t CarSpeed;
	uint32_t PressureTireFL;
	uint32_t PressureTireFR;
	uint32_t PressureTireBL;
	uint32_t PressureTireBR;
	uint32_t InverterTemperature;
	uint32_t TemperatureBatteryLV;
};

static const struct json_obj_descr data_descr[] = {
  JSON_OBJ_DESCR_PRIM(struct data, TensionBatteryHV, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct data, AmperageBatteryHV, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct data, TemperatureBatteryHV, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct data, EnginePower, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct data, EngineTemperature, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct data, EngineAngularSpeed, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct data, CarSpeed, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct data, PressureTireFL, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct data, PressureTireFR, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct data, PressureTireBL, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct data, PressureTireBR, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct data, InverterTemperature, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct data, TemperatureBatteryLV, JSON_TOK_NUMBER),
};
		

void Data_Sender() 
{
	
	
	struct data myData;
	
	while(true)
	{
		myData.TensionBatteryHV= sys_rand32_get()%600;
		myData.AmperageBatteryHV=sys_rand32_get()%200;
		myData.TemperatureBatteryHV=sys_rand32_get()%80;
		myData.EnginePower=sys_rand32_get()%35000;
		myData.EngineTemperature=sys_rand32_get()%80;
		myData.EngineAngularSpeed=sys_rand32_get()%2300;
		myData.CarSpeed=sys_rand32_get()%120;
		myData.PressureTireFL=sys_rand32_get()%6;
		myData.PressureTireFR=sys_rand32_get()%6;
		myData.PressureTireBL=sys_rand32_get()%6;
		myData.PressureTireBR=sys_rand32_get()%6;
		myData.InverterTemperature=sys_rand32_get()%80;
		myData.TemperatureBatteryLV=sys_rand32_get()%80;

		
		if(context.ip_assigned)
		{
			char * memPtr = k_heap_alloc(&messageHeap,configFile.TelemetyDataSize,K_NO_WAIT);



			if(memPtr != NULL)			//memory alloc success
			{

				json_obj_encode_buf(data_descr,ARRAY_SIZE(data_descr),&myData,memPtr,configFile.TelemetyDataSize);		//put json in allocated memory
				k_queue_append(&udpQueue,memPtr);

			}
			else //mem alloc fail
			{
				LOG_ERR("data sender memory allocation failed");
			}
		}

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



