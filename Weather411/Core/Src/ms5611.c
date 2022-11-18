#include "ms5611.h"
#include "main.h"
#include "UartRingbuffer.h"
#include "stdio.h"


// sensor calibration coefficients
unsigned int gC[8];
unsigned long gD1;
unsigned long gD2;
long gdT;
long gTEMP;
long long gOFF;
long long gSENS;
long gP;
long gnTempC;

long t;

uint8_t bytes[3];

char buffer[100];





//MS5611 sensor

void MS5611_Init(void) {




    sns_Reset();

    sns_ReadCoeffs();

    sns_TriggerTemperatureSample();

}



/// Read the calibration coefficients from the MS5611 pressure sensor

void sns_ReadCoeffs(void) {
    unsigned char cnt;
    for (cnt = 0; cnt < 8; cnt++) {

    	user_spi_read(0xA0 + cnt * 2, bytes, 2);

    	gC[cnt] = bytes[0] << 8 | bytes[1];


//        spi_Send8(0xA0 + cnt * 2);
//        gC[cnt] = spi_Receive16();



    }

    sprintf(buffer, "MS5611 Calibration: %i %i %i %i %i %i %i %i\r\n",
			gC[0],
			gC[1],
			gC[2],
			gC[3],
			gC[4],
			gC[5],
			gC[6],
			gC[7]);


	  Uart_SendString(buffer, pc_uart);
}

void sns_Reset(void) {

//    spi_Send8(0x1E);
    user_spi_write(0x1E, bytes, 0);

    HAL_Delay(100);



}


// Trigger a pressure sample with max oversampling rate

void sns_TriggerPressureSample(void) {

//    spi_Send8(0x48);
    user_spi_write(0x48, bytes, 0);


}

// Trigger a temperature sample with max oversampling rate

void sns_TriggerTemperatureSample(void) {

//    spi_Send8(0x58);
    user_spi_write(0x58, bytes, 0);


}


// Read a raw temperature sample

unsigned long sns_ReadTemperatureSample(void) {

    unsigned long w;

//    spi_Send8(0x00);
//    w = spi_Receive24();

    user_spi_read(0x00, bytes, 3);
    w = bytes[0] << 16 | bytes[1] << 8 | bytes[2];

    gD2 = w;
    return w;
}




// Read a raw pressure sample

unsigned long sns_ReadPressureSample(void) {

    unsigned long w;

//    spi_Send8(0x00);
//    w = spi_Receive24();
    user_spi_read(0x00, bytes, 3);
    w = bytes[0] << 16 | bytes[1] << 8 | bytes[2];

    gD1 = w;
    return w;
}

long sns_CalculateTemperatureCx10(void) {

    gdT = (long) gD2 - (((long) gC[5]) << 8);
    t = 2000 + ((gdT * ((long long) gC[6])) >> 23);

    gnTempC = (t >= 0 ? (t + 5) / 10 : (t - 5) / 10);
    return gnTempC;
}

long sns_CalculatePressurePa(void) {
    long pa;

    gOFF = (((long long) gC[2]) << 16) + ((((long long) gC[4])*(long long) gdT) >> 7);
    gSENS = (((long long) gC[1]) << 15) + ((((long long) gC[3])*(long long) gdT) >> 8);
    pa = (long) ((((((long long) gD1) * gSENS) >> 21) - gOFF) >> 15);
    gP = pa;
    return pa;
}



uint32_t getPressure(void){

	sns_TriggerTemperatureSample();
	HAL_Delay(10);
	sns_ReadTemperatureSample();
	sns_CalculateTemperatureCx10();

    sns_TriggerPressureSample();
	HAL_Delay(10);
	sns_ReadPressureSample();
	sns_CalculatePressurePa();

	return (uint32_t)gP;

}

long getTemperature() {
    return gnTempC;

}

long getTempCx100() {
    return t;

}

unsigned int getgCal(int index) {
    return gC[index];
}








