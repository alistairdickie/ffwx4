#include "measure_buffer.h"

#define measureBufferSize 64 //12 minutes of 11.25s measurements

FFWX_Measurement measurement[measureBufferSize];
uint8_t addIndex;
//uint8_t oldestIndex=0;

uint8_t sendIndex;
//uint8_t storeIndex=0;

uint8_t numToSend;
//uint8_t numToStore=0;

void initMeasureBuffer(){
	addIndex = 0;
	sendIndex = 0;
	numToSend = 0;
}



void addMeasurement(FFWX_Measurement * newMeasurement){



	if(numToSend >= measureBufferSize){


		//adjust the number still to store/send based on what has happened with the oldest one.
		if(measurement[sendIndex].sentStatus != 2){//we have not send the oldest one, so overwrite it and increase the send index to the next one
			numToSend--;
			sendIndex++;

		}
//		if(!measurement[oldestIndex].stored){//we have not stored the oldest one
//			numToStore--;
//			storeIndex++;
//		}

		//discard the oldest one
//		oldestIndex++;
//		if(oldestIndex > measureBufferSize){
//			oldestIndex = 0;
//		}

		if(sendIndex >= measureBufferSize){
			sendIndex = 0;
		}

//		if(storeIndex > measureBufferSize){
//			storeIndex = 0;
//		}


	}


	measurement[addIndex].index = newMeasurement->index;
	measurement[addIndex].unix_time.seconds = newMeasurement->unix_time.seconds;
	measurement[addIndex].unix_time.ms = newMeasurement->unix_time.ms;
	measurement[addIndex].mph = newMeasurement->mph;
	measurement[addIndex].direction = newMeasurement->direction;
	measurement[addIndex].mphMax = newMeasurement->mphMax;
	measurement[addIndex].mphMin = newMeasurement->mphMin;
	measurement[addIndex].pressure = newMeasurement->pressure;
	measurement[addIndex].temperature = newMeasurement->temperature;
	measurement[addIndex].humidity = newMeasurement->humidity;
	measurement[addIndex].battery = newMeasurement->battery;
	measurement[addIndex].csq = newMeasurement->csq;
	measurement[addIndex].sentStatus = 0;
//	measurement[addIndex].stored = 0;
	numToSend++;
//	numToStore++;

	//add a the next new one to the next position
	addIndex++;
	if(addIndex >= measureBufferSize){
		addIndex = 0;
	}
}

uint8_t numMeasurementsToSend(void){
	return numToSend;
}

void getMeasurementToSend(FFWX_Measurement * measure){
	if(numToSend && measurement[sendIndex].sentStatus != 2){
		measure->index = measurement[sendIndex].index;
		measure->unix_time.seconds = measurement[sendIndex].unix_time.seconds;
		measure->unix_time.ms = measurement[sendIndex].unix_time.ms;
		measure->mph = measurement[sendIndex].mph;
		measure->direction = measurement[sendIndex].direction;
		measure->mphMax = measurement[sendIndex].mphMax;
		measure->mphMin = measurement[sendIndex].mphMin;
		measure->pressure = measurement[sendIndex].pressure;
		measure->temperature = measurement[sendIndex].temperature;
		measure->humidity = measurement[sendIndex].humidity;
		measure->battery = measurement[sendIndex].battery;
		measure->csq = measurement[sendIndex].csq;
		measure->sentStatus = measurement[sendIndex].sentStatus;
		measurement[sendIndex].sentStatus = 1;
//		measure->stored = measurement[sendIndex].stored;
	}


}

//uint8_t getMeasurementToStore(FFWX_Measurement * measure){
//	if(numToStore == 0){
//		return 0;
//	}
//	else{
//
//		measure->index = measurement[storeIndex].index,
//		measure->unix_time.seconds = measurement[storeIndex].unix_time.seconds,
//		measure->unix_time.ms = measurement[storeIndex].unix_time.ms,
//		measure->mph = measurement[storeIndex].mph,
//		measure->direction = measurement[storeIndex].direction,
//		measure->mphMax = measurement[storeIndex].mphMax,
//		measure->mphMin = measurement[storeIndex].mphMin,
//		measure->pressure = measurement[storeIndex].pressure,
//		measure->temperature = measurement[storeIndex].temperature,
//		measure->humidity = measurement[storeIndex].humidity,
//		measure->battery = measurement[storeIndex].battery,
//		measure->csq = measurement[storeIndex].csq,
//		measure->sent = measurement[storeIndex].sent,
//		measure->stored = measurement[storeIndex].stored;
//	}
//
//	return numToStore;
//}

uint8_t confirmSent(uint32_t index){
	if(index == measurement[sendIndex].index){
		measurement[sendIndex].sentStatus = 2;

//		if(measurement[sendIndex].stored){
//			oldestIndex++;
//		}


		sendIndex++;
		numToSend--;

		if(sendIndex >= measureBufferSize){
			sendIndex = 0;
		}
//		if(oldestIndex >= measureBufferSize){
//			oldestIndex = 0;
//		}
		return 1;

	}
	return 0;

}

//uint8_t confirmStored(uint32_t index){
//	if(index == measurement[storeIndex].index){
//		measurement[storeIndex].stored = 1;
//
//		if(measurement[sendIndex].sent){
//			oldestIndex++;
//		}
//
//
//		storeIndex++;
//		numToStore--;
//		if(storeIndex >= measureBufferSize){
//			storeIndex = 0;
//		}
//		if(oldestIndex >= measureBufferSize){
//			oldestIndex = 0;
//		}
//		return 1;
//	}
//	return 0;
//}


void measurementToParams(FFWX_Measurement * measurement, char * buffer){
		sprintf(buffer,
			"?s=%u"
			"&p=%lu"
			"&e=%lu%03u"
			"&w=%.1f"
			"&d=%.0f"
			"&x=%u"
			"&n=%u"
			"&q=%lu"
			"&t=%.1f"
			"&h=%.1f"
			"&b=%.2f"
			"&c=%u",
			getSettings()->SID,
			measurement->index,
			measurement->unix_time.seconds,measurement->unix_time.ms,
			measurement->mph,
			measurement->direction,
			measurement->mphMax,
			measurement->mphMin,
			measurement->pressure,
			measurement->temperature,
			measurement->humidity,
			measurement->battery,
			measurement->csq
			);
}

void measurementToSentance(FFWX_Measurement * measurement, char * buffer){
	sprintf(buffer, "$FFWX,1,"
			"%lu,"
			"%lu%03u,"
			"%.1f,"
			"%.0f,"
			"%u,"
			"%u,"
			"%li,"
			"%.1f,"
			"%.1f,"
			"%.2f,"
			"%u"
			"\r\n",
			measurement->index,
			measurement->unix_time.seconds,
			measurement->unix_time.ms,
			measurement->mph,
			measurement->direction,
			measurement->mphMax,
			measurement->mphMin,
			measurement->pressure,
			measurement->temperature,
			measurement->humidity,
			measurement->battery,
			measurement->csq);
}

