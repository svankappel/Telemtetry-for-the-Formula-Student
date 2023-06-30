#ifndef __DATA_SENDER_H
#define __DATA_SENDER_H


void Data_Sender();

void Task_Data_Sender_Init( void );

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
#endif /*__DATA_SENDER_H*/