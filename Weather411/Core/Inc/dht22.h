#define DHT22_PORT GPIOB
#define DHT22_PIN GPIO_PIN_9

#include "stm32f4xx_hal.h"
#include "main.h"

void DHT22_Start (void);
uint8_t DHT22_Check_Response (void);
uint8_t DHT22_Read (void);
void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void Set_Pin_Input (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
uint8_t get_DHT_Temp_Hum (float * temperature, float * humidity);

