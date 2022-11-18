#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_

#include "FileHandling.h"
#include "UartRingbuffer.h"
#include "global.h"

#define UART_LINE_BUFFER_DIM 1024

typedef struct{

	char MEI[20];
	uint16_t SID;
    char URL[160];
    uint8_t LOG;
    float BCL;
    uint8_t RSH;
    uint16_t AVN;


} SETTINGS;


typedef struct {

	char line[UART_LINE_BUFFER_DIM];
	uint8_t numLines;
	uint8_t lineWriteIndex;
	uint8_t lineReadIndex;

}UART_LineBuffer;

typedef enum
{
	SDSTATE_INIT=0,
	SDSTATE_OK=1,
	SDSTATE_ERR=2,
	SDSTATE_RECOVER=3,

} SDSTATE;



void SDcard_Init();
void defaultSettings(void);
void openSettingsFromSD(void);

SETTINGS * getSettings(void);
void printSettings(void);
void settingsLineProcessor(char * line, uint8_t saveSD, uint8_t saveServer);

void pc_uart_processChar(char c);

void UART_lineBufferAdd(UART_LineBuffer *uLineBuffer, char c);
uint8_t UART_lineBufferNumLines(UART_LineBuffer *uLineBuffer);
void UART_lineBufferGetLine(UART_LineBuffer *uLineBuffer, char * line);
void UART_processLines(void);

void Log(char * line, uint8_t logType);

void setSaveFlag(uint8_t flag);
SDSTATE getSDState();

void saveSettingsToSD(void);

uint8_t getUpdateStationListFlag(void);

void setUpdateStationListFlag(uint8_t flag);

void settingsToParams(char * buffer);

void shutdown();

void scheduleShutdown(uint16_t ms);

uint8_t checkShutdown(void);

uint8_t getShutdownScheduledFlag();

void checkWriteLogBuffer(void);

void forceWriteLogBuffer(void);

void checkSDError(void);


#endif /* INC_SETTINGS_H_ */
