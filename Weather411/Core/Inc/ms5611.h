#include "stm32f4xx_hal.h"

////spi
//
//unsigned long spi_Receive24(void);
//unsigned int spi_Receive16(void);
//unsigned char spi_Send8(unsigned char out);

//sensor
void sns_TriggerPressureSample(void);
void sns_TriggerTemperatureSample(void);

void MS5611_Init(void);
void sns_Reset(void);
void sns_ReadCoeffs(void);


uint32_t getPressure(void);
long getTemperature(void);
long getTempCx100(void);

unsigned long  sns_ReadPressureSample(void);
unsigned long  sns_ReadTemperatureSample(void);
long  sns_CalculateTemperatureCx10(void);
long  sns_CalculatePressurePa(void);

unsigned int getgCal(int index);
