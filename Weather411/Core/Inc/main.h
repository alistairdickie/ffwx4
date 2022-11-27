/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

int8_t user_spi_read(uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
int8_t user_spi_write(uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
void delay_us (uint16_t us);
void timer2_ms_restart(void);
uint32_t timer2_ms_get(void);
void timer4_ms_restart(void);
uint32_t timer4_ms_get(void);
void timer5_ms_restart(void);
uint32_t timer5_ms_get(void);
void timer10_ms_restart(void);
uint32_t timer10_ms_get(void);
RTC_HandleTypeDef * getHRTC(void);



/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC1_IN0_WINDDIR_Pin GPIO_PIN_0
#define ADC1_IN0_WINDDIR_GPIO_Port GPIOA
#define ADC1_IN1_VSENSE_Pin GPIO_PIN_1
#define ADC1_IN1_VSENSE_GPIO_Port GPIOA
#define GPIO_OUT_4_1EN_Pin GPIO_PIN_4
#define GPIO_OUT_4_1EN_GPIO_Port GPIOA
#define GPIO_IN_POWERSTAT_Pin GPIO_PIN_0
#define GPIO_IN_POWERSTAT_GPIO_Port GPIOB
#define GPIO_OUT_DTR_Pin GPIO_PIN_1
#define GPIO_OUT_DTR_GPIO_Port GPIOB
#define GPIO_IN_RI_Pin GPIO_PIN_2
#define GPIO_IN_RI_GPIO_Port GPIOB
#define GPIO_IN_DCD_Pin GPIO_PIN_10
#define GPIO_IN_DCD_GPIO_Port GPIOB
#define GPIO_OUT_RESET_Pin GPIO_PIN_12
#define GPIO_OUT_RESET_GPIO_Port GPIOB
#define GPIO_OUT_PWRKEY_Pin GPIO_PIN_13
#define GPIO_OUT_PWRKEY_GPIO_Port GPIOB
#define GPIO_OUT_LED_Pin GPIO_PIN_14
#define GPIO_OUT_LED_GPIO_Port GPIOB
#define SD_CD_Pin GPIO_PIN_10
#define SD_CD_GPIO_Port GPIOA
#define GPIO_OUT_U_CS_Pin GPIO_PIN_6
#define GPIO_OUT_U_CS_GPIO_Port GPIOB
#define GPIO_IN_WINDCOUNT_Pin GPIO_PIN_8
#define GPIO_IN_WINDCOUNT_GPIO_Port GPIOB
#define GPIO_IN_WINDCOUNT_EXTI_IRQn EXTI9_5_IRQn
#define GPIO_TH_DATA_Pin GPIO_PIN_9
#define GPIO_TH_DATA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
