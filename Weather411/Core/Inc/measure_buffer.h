#ifndef INC_MEASURE_BUFFER_H_
#define INC_MEASURE_BUFFER_H_

#include <settings.h>
#include "stm32f4xx_hal.h"
#include "stdio.h"


typedef struct {

	uint32_t seconds;
	uint16_t ms;

}EpochTime;


typedef struct
{
  uint32_t index;
  EpochTime unix_time;
  float mph;
  float direction;
  uint16_t mphMin;
  uint16_t mphMax;
  uint32_t pressure;
  float temperature;
  float humidity;
  float battery;
  uint8_t csq;
  uint8_t sentStatus;
//  uint8_t stored;


}FFWX_Measurement;

void initMeasureBuffer();


void measurementToParams(FFWX_Measurement * measurement, char * buffer);
void measurementToSentance(FFWX_Measurement * measurement, char * buffer);
void addMeasurement(FFWX_Measurement * newMeasurement);
uint8_t numMeasurementsToSend(void);
void getMeasurementToSend(FFWX_Measurement * measure);
//uint8_t getMeasurementToStore(FFWX_Measurement * measure);
uint8_t confirmSent(uint32_t index);
//uint8_t confirmStored(uint32_t index);

#endif /* INC_MEASURE_BUFFER_H_ */
