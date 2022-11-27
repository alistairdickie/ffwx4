
#include <settings.h>
#include "util.h"
#include "timeUtil.h"
#include "sim7600.h"

#define LOG_BUFFER_SIZE 4096

SETTINGS settings;
FRESULT fresult1;



char * settingsFile = "settings.txt";

char buffer2[200]; //settings file line buffer

UART_LineBuffer uart_lineBuffer;
char line1[200];

char logdir[10];
char logfile[30];

uint8_t saveFlag;

uint8_t updateStationListFlag;

SDSTATE sdState;

uint8_t settingsValid;

uint8_t shutdownScheduledFlag;
uint16_t shutdownScheduledTimeout;

char logBufferA[LOG_BUFFER_SIZE];
char logBufferB[LOG_BUFFER_SIZE];
uint16_t logBufferA_position;
uint16_t logBufferB_position;
uint8_t currentLogBuffer;
uint8_t logBufferReady;




void SDcard_Init(void){

	sdState = SDSTATE_INIT;
	settings.URL[0] = 0;
	settings.SID = 0;
	saveFlag = 0;
	settingsValid = 0;


	Uart_SendString("Mounting SD Card...\r\n", pc_uart);
	fresult1 = Mount_SD("/");
	if (fresult1 != FR_OK){
		sdState = SDSTATE_ERR;
		defaultSettings();
		return;

	}
	fresult1 = Check_SD_Space();
	if (fresult1 != FR_OK){
		sdState = SDSTATE_ERR;
		defaultSettings();
		return;
	}

	sdState = SDSTATE_OK;


}

void checkSDError(void){
//	switch(sdState){
//
//		case SDSTATE_INIT:
//			SDcard_Init();
//			break;
//		case SDSTATE_OK:
//			break;
//		case SDSTATE_ERR:
//			sdState = SDSTATE_RECOVER;
//			break;
//		case SDSTATE_RECOVER:
//			MX_FATFS_DeInit();
//			MX_FATFS_Init();
//			sdState = SDSTATE_INIT;
//			break;
//
//	}
}

void defaultSettings(void){
	//check if all key settings are set. If not, set to defaults.
	if(strlen(settings.URL) < 8){
		strcpy(settings.URL, "freeflightwx.com/test/up4.php");
		setSaveFlag(1);
		//setUpdateStationListFlag(1);

	}

	if(!settings.SID){
		settings.SID = 1234;
		setSaveFlag(1);
		//setUpdateStationListFlag(1);

	}

	if(!settings.BCL){
		settings.BCL = 0.0039487; //Volts in = ADCValue * BCL Multiplier ::: BCL Multiplier Default = 3.3V full value/4095 ADC full value * (39ohm + 10ohm ) / 10ohm
		setSaveFlag(1);
		//setUpdateStationListFlag(1);

	}

	if(!settings.RSH){
		settings.RSH = 9;
		setSaveFlag(1);
		//setUpdateStationListFlag(1);

	}

	if(!settings.AVN){
		settings.AVN = 5;
		setSaveFlag(1);
		//setUpdateStationListFlag(1);

	}

}

void openSettingsFromSD(void){


	Uart_SendString("Opening Settings\r\n", pc_uart);

	fresult1 = File_O_Open(settingsFile);
	if (fresult1 != FR_OK){
		sdState = SDSTATE_ERR;
		defaultSettings();
		return;

	}


	TCHAR * result;

	while((result = File_O_Read_Line(buffer2))){
		settingsLineProcessor(buffer2, 0, 0);
	}
	fresult1 = File_O_Close();
	if (fresult1 != FR_OK){
		sdState = SDSTATE_ERR;
		defaultSettings();
		return;

	}

//	printSettings();

	defaultSettings();

	printSettings();
	settingsValid = 1;//prevents calling for settings from the station server.

}

void setSaveFlag(uint8_t flag){
	saveFlag = flag;

}

SDSTATE getSDState(){
	return sdState;
}



void saveSettingsToSD(void){

	if(saveFlag){


		if(sdState == SDSTATE_OK){

			Uart_SendString("Saving Settings to SD Card\r\n", pc_uart);


			fresult1 = File_O_Open(settingsFile);

			if (fresult1 != FR_OK){
				sdState = SDSTATE_ERR;
			}

			File_O_Print("#FreeFlightWx Settings\r\n");
			sprintf(buffer2, "MEI=%s\r\n", settings.MEI);
			File_O_Print(buffer2);
			sprintf(buffer2, "SID=%i\r\n", settings.SID);
			File_O_Print(buffer2);
			sprintf(buffer2, "URL=%s\r\n", settings.URL);
			File_O_Print(buffer2);
			sprintf(buffer2, "LOG=%i\r\n", settings.LOG);
			File_O_Print(buffer2);
			sprintf(buffer2, "BCL=%.7f\r\n", settings.BCL);
			File_O_Print(buffer2);
			sprintf(buffer2, "RSH=%i\r\n", settings.RSH);
			File_O_Print(buffer2);
			sprintf(buffer2, "AVN=%i\r\n", settings.AVN);
			File_O_Print(buffer2);
			fresult1 = File_O_Close();

			if (fresult1 != FR_OK){
				sdState = SDSTATE_ERR;
			}
		}

		saveFlag = 0;
	}



}
void printSettings(void){

	Uart_SendString("Print Settings ...\r\n", pc_uart);

	Uart_SendString("#FreeFlightWx Settings\r\n", pc_uart);
	sprintf(buffer2, "MEI=%s\r\n", settings.MEI);
	Uart_SendString(buffer2, pc_uart);
	sprintf(buffer2, "SID=%i\r\n", settings.SID);
	Uart_SendString(buffer2, pc_uart);
	sprintf(buffer2, "URL=%s\r\n", settings.URL);
	Uart_SendString(buffer2, pc_uart);
	sprintf(buffer2, "LOG=%i\r\n", settings.LOG);
	Uart_SendString(buffer2, pc_uart);
	sprintf(buffer2, "BCL=%.7f\r\n", settings.BCL);
	Uart_SendString(buffer2, pc_uart);
	sprintf(buffer2, "RSH=%i\r\n", settings.RSH);
	Uart_SendString(buffer2, pc_uart);
	sprintf(buffer2, "AVN=%i\r\n", settings.AVN);
	Uart_SendString(buffer2, pc_uart);


}


SETTINGS * getSettings(void){
	return &settings;
}

uint8_t getUpdateStationListFlag(void){
	return updateStationListFlag;
}

void setUpdateStationListFlag(uint8_t flag){
	updateStationListFlag = flag;
//	scheduleSendParamsToUp();
}


void settingsLineProcessor(char * line, uint8_t saveSD, uint8_t saveServer){

//	Uart_sendstring("X\r\n", pc_uart);
//	Uart_SendString(line, pc_uart);

    if(line[0] == '#') {
    	return;
    }

    else if(!strncmp(line, "URL=", 4)){
//    	Uart_sendstring("TEST", pc_uart);
    	uint8_t i = 4;

    	//do a special copy
    	while (line[i] && line[i] != '\r' && line[i] != '\n'){
    		settings.URL[i-4] = line[i];
    		i++;
    	}
    	settings.URL[i-4] = 0;//add a null character

    	setSaveFlag(saveSD);
    	setUpdateStationListFlag(saveServer);

    }

    else if(!strncmp(line, "SID=", 4)){

    	settings.SID=util_atoi_n(line + 4 ,20);

    	setSaveFlag(saveSD);
    	setUpdateStationListFlag(saveServer);

    }
    else if(!strncmp(line, "BCL=", 4)){

		settings.BCL=(float)(atof(line + 4));

		setSaveFlag(saveSD);
		setUpdateStationListFlag(saveServer);

	}

    else if(!strncmp(line, "LOG=", 4)){

		settings.LOG=util_atoi_n(line + 4 ,20);

		setSaveFlag(saveSD);
		setUpdateStationListFlag(saveServer);


	}

    else if(!strncmp(line, "RSH=", 4)){

		settings.RSH=util_atoi_n(line + 4 ,20);

		setSaveFlag(saveSD);
		setUpdateStationListFlag(saveServer);

	}
    else if(!strncmp(line, "AVN=", 4)){

		settings.AVN=util_atoi_n(line + 4 ,20);

		setSaveFlag(saveSD);
		setUpdateStationListFlag(saveServer);

	}
    else if(!strncmp(line, "VLD=", 4)){

    	settingsValid=util_atoi_n(line+ 4 ,20);

	}

    else if(!strncmp(line, "CLK=", 4)){

		setTime(line + 4, TIMESTATE_OK_SERVER);

	}
    else if(!strncmp(line, "GET", 3 )){

		printSettings();

	}

    else if(!strncmp(line, "SAV", 3 )){

    	setSaveFlag(saveSD);
		setUpdateStationListFlag(saveServer);

	}


    else if(!strncmp(line, "RST", 3 )){

		scheduleShutdown(20000);

	}

    else if(!strncmp(line, "HOLD", 4 )){

		toggleHold();

	}

    else if(!strncmp(line, "FMT", 3 )){

    	Uart_SendString("Formatting SD Card...\r\n", pc_uart);
    	Unmount_SD("/");
    	Mount_SD("/");
    	Format_SD();
    	Check_SD_Space();

   		//todo - more shutdown stuff like saving settings again
    	scheduleShutdown(20000);

   	}



    return;
}


void pc_uart_processChar(char c){
	UART_lineBufferAdd(&uart_lineBuffer, c);
	UART_processLines();
}

void UART_lineBufferAdd(UART_LineBuffer *uLineBuffer, char c){
	if(!(uLineBuffer->numLines && uLineBuffer->lineWriteIndex == uLineBuffer->lineReadIndex)){//buffer is not full (assuming that there is at least one end line in there)
		uLineBuffer->line[uLineBuffer->lineWriteIndex++] = c;
		if (uLineBuffer->lineWriteIndex == LINE_BUFFER_DIM) {
			uLineBuffer->lineWriteIndex = 0;
		}
		if(c == '\n'){
			uLineBuffer->numLines++;
		}
	}
}

uint8_t UART_lineBufferNumLines(UART_LineBuffer *uLineBuffer){
	return uLineBuffer->numLines;

}

void UART_lineBufferGetLine(UART_LineBuffer *uLineBuffer, char * line){
	if(uLineBuffer->numLines){

		char c = 0;
		int i = 0;
		while(c != '\n'){
			c = uLineBuffer->line[uLineBuffer->lineReadIndex++];
			line[i] = c;
			i++;

			if (uLineBuffer->lineReadIndex == LINE_BUFFER_DIM) {
				uLineBuffer->lineReadIndex = 0;
			}
		}

		uLineBuffer->numLines--;
		line[i] = 0;//add eol char
	}
}



void UART_processLines(void)
{


    if(UART_lineBufferNumLines(&uart_lineBuffer)){//we have a line ready to read


    	UART_lineBufferGetLine(&uart_lineBuffer, line1);

       if(line1[0] == '$') {
    	   NMEAProcessor(line1);
       }



    }
}

void Log(char * line, uint8_t logType){
//	unsigned int t1,t2;

	uint16_t lineIndex = 0;


	if(logType <= settings.LOG){
		while(line[lineIndex]){
			if(currentLogBuffer){//addtobufferB
				logBufferB[logBufferB_position] = line[lineIndex];
				logBufferB_position++;
				if(logBufferB_position == LOG_BUFFER_SIZE){
					currentLogBuffer = 0;//switch to A for the next char
					logBufferReady = 1;//ready to empty the not current one;
					logBufferB_position = 0; //reset for B for next time
				}

			}
			else{//addtoBufferA
				logBufferA[logBufferA_position] = line[lineIndex];
				logBufferA_position++;
				if(logBufferA_position == LOG_BUFFER_SIZE){
					currentLogBuffer = 1;//switch to B for the next char
					logBufferReady = 1;//ready to empty the not current one;
					logBufferA_position = 0;//rest for A next time
				}


			}
			lineIndex++;

		}




	}
	else{
//		Uart_SendString("LOG setting prevents logging\r\n", pc_uart);
	}

}

void checkWriteLogBuffer(void){
	if(logBufferReady){
		if(sdState == SDSTATE_OK){


//			timer5_ms_restart();


			getDateCharFileName(logfile);
			getYearCharDirName(logdir);

			/**** check whether the dir exists or not ****/
			FILINFO fno;
			fresult1 = f_stat (logdir, &fno);
			if (fresult1 != FR_OK){
				fresult1 = Create_Dir(logdir);
				if (fresult1 != FR_OK){
					sdState = SDSTATE_ERR;
				}
			}
	//			t1 = (unsigned int) timer5_ms_get();

			if(currentLogBuffer){//buffering into B, so A must be ready
				fresult1 = Update_File (logfile, logBufferA, LOG_BUFFER_SIZE );
				if (fresult1 == FR_OK){
					Uart_SendString("Wrote Log Buffer A\r\n", pc_uart);
//					Uart_SendString("###\r\n", pc_uart);
//					Uart_SendString(logBufferA, pc_uart);
//					Uart_SendString("\r\n###\r\n", pc_uart);
				}
				else{
					Uart_SendString("SD Card Write Error A\r\n", pc_uart);

					sdState = SDSTATE_ERR;
				}


			}
			else{
				fresult1 = Update_File (logfile, logBufferB, LOG_BUFFER_SIZE );
				if (fresult1 == FR_OK){
					Uart_SendString("Wrote Log Buffer B\r\n", pc_uart);
//					Uart_SendString("###\r\n", pc_uart);
//					Uart_SendString(logBufferB, pc_uart);
//					Uart_SendString("\r\n###\r\n", pc_uart);
				}
				else{
					Uart_SendString("SD Card Write Error B\r\n", pc_uart);
					sdState = SDSTATE_ERR;
				}
			}



	//			t2 = (unsigned int) timer5_ms_get();
	//			sprintf(buffer, "SD Write Time %ul %ul\r\n", t1, t2);
	//			Uart_SendString(buffer, pc_uart);



		}
		else{
			Uart_SendString("Logging disabled - SD Card Error\r\n", pc_uart);

		}
		logBufferReady = 0;
	}
}

void forceWriteLogBuffer(void){

	if(sdState == SDSTATE_OK){


//			timer5_ms_restart();


		getDateCharFileName(logfile);
		getYearCharDirName(logdir);

		/**** check whether the dir exists or not ****/
		FILINFO fno;
		fresult1 = f_stat (logdir, &fno);
		if (fresult1 != FR_OK){
			fresult1 = Create_Dir(logdir);
			if (fresult1 != FR_OK){
				sdState = SDSTATE_ERR;
			}
		}
//			t1 = (unsigned int) timer5_ms_get();

		if(currentLogBuffer){//buffering into B, so use that one
			fresult1 = Update_File (logfile, logBufferB, logBufferB_position );
			if (fresult1 == FR_OK){
				Uart_SendString("Flush Log Buffer B\r\n", pc_uart);
//					Uart_SendString("###\r\n", pc_uart);
//					Uart_SendString(logBufferA, pc_uart);
//					Uart_SendString("\r\n###\r\n", pc_uart);
			}
			else{
				Uart_SendString("SD Card Write Error B\r\n", pc_uart);

				sdState = SDSTATE_ERR;
			}


		}
		else{
			fresult1 = Update_File (logfile, logBufferA, logBufferA_position );
			if (fresult1 == FR_OK){
				Uart_SendString("Wrote Log Buffer A\r\n", pc_uart);
//					Uart_SendString("###\r\n", pc_uart);
//					Uart_SendString(logBufferB, pc_uart);
//					Uart_SendString("\r\n###\r\n", pc_uart);
			}
			else{
				Uart_SendString("SD Card Write Error A\r\n", pc_uart);
				sdState = SDSTATE_ERR;
			}
		}



//			t2 = (unsigned int) timer5_ms_get();
//			sprintf(buffer, "SD Write Time %ul %ul\r\n", t1, t2);
//			Uart_SendString(buffer, pc_uart);



	}
	else{
		Uart_SendString("Logging disabled - SD Card Error\r\n", pc_uart);

	}
	logBufferReady = 0;

}

void settingsToParams(char * buffer){
//	char MEI[20];
//		uint16_t SID;
//	    char URL[160];
//	    uint8_t LOG;
//	    float BCL;
//	    uint8_t RSH;

		sprintf(buffer,
			"?MEI=%s"
			"&SID=%u"
			"&URL=%s"
			"&LOG=%u"
			"&BCL=%.7f"
			"&RSH=%u"
			"&AVN=%u"
			"&VLD=%u"
			"&VER=%s_%s",
			settings.MEI,
			settings.SID,
			settings.URL,
			settings.LOG,
			settings.BCL,
			settings.RSH,
			settings.AVN,
			settingsValid,
			MAJOR_VERSION,BUILD_NUMBER
			);
}

void shutdown(){
	Uart_SendString("Shutdown...\r\n", pc_uart);
	forceWriteLogBuffer();
	//todo - more shutdown stuff
	NVIC_SystemReset();
}

void scheduleShutdown(uint16_t ms){
	Uart_SendString("Shutdown scheduled\r\n", pc_uart);
	shutdownScheduledFlag = 1;
	timer4_ms_restart();
	shutdownScheduledTimeout = ms;

}

uint8_t checkShutdown(void){
	if(shutdownScheduledFlag && timer4_ms_get() >= shutdownScheduledTimeout){
		Uart_SendString("Shutdown timer reached\r\n", pc_uart);
		shutdown();
	}

	return shutdownScheduledFlag;

}

uint8_t getShutdownScheduledFlag(){
	return shutdownScheduledFlag;
}

