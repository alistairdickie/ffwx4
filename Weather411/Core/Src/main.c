/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "UartRingbuffer.h"
#include "stdio.h"
#include "ms5611.h"
#include "dht22.h"
#include "math.h"
#include "timeUtil.h"
#include "sim7600.h"
#include "measure_buffer.h"
#include "global.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */



/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */



//#define pc_uart &huart2
//#define sim_uart &huart1
//#define PERIOD 2.25
//#define AVG_NUM 5
#define deg2rad 0.0174532925
#define rad2deg 57.2957795
#define H_TEST 99


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

IWDG_HandleTypeDef hiwdg;

RTC_HandleTypeDef hrtc;

SD_HandleTypeDef hsd;
DMA_HandleTypeDef hdma_sdio_rx;
DMA_HandleTypeDef hdma_sdio_tx;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim9;
TIM_HandleTypeDef htim10;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

GPIO_PinState simStatus;
uint32_t adcValues[2];  // to store the adc values
char buffer[100];

uint32_t powerOnIndex = 1;

float temperature = 0;
float humidity = 0;
//long pressure = 0;

uint16_t windCount;
uint16_t mph;
float direction;
float battery;

//for averaging
uint8_t avgCount;
float vectorX;
float vectorY;

uint16_t maxSpeed = 0;
uint16_t minSpeed = 1000;
float avgSpeed;
float avgDirection;

float batSum;


FFWX_Measurement currentMeasurement;

uint8_t newMeasurement = 0;

uint8_t lastHour = H_TEST;
uint8_t thisHour = H_TEST;

EpochTime epochTime;

uint16_t avgNum = 5;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SDIO_SD_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM9_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM5_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM10_Init(void);
/* USER CODE BEGIN PFP */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SDIO_SD_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_FATFS_Init();
  MX_SPI1_Init();
  MX_RTC_Init();
  MX_TIM9_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_IWDG_Init();
  MX_TIM10_Init();
  /* USER CODE BEGIN 2 */






	Ringbuf_Init();  // init the ring buffer for UART

	Uart_SendString("\r\nStarting FreeFlightWX v", pc_uart);
	Uart_SendString(MAJOR_VERSION, pc_uart);
	Uart_SendString("_", pc_uart);
	Uart_SendString(BUILD_NUMBER, pc_uart);
	Uart_SendString("\r\n", pc_uart);

	initMeasureBuffer();

	SDcard_Init();

	openSettingsFromSD();

	initTime();


	HAL_TIM_Base_Start(&htim9);

	HAL_ADC_Start_DMA(&hadc1, adcValues, 2);  // start the adc in dma mode. adcValues is the buffer, where the adc values are going to store



	//startup sensors

	MS5611_Init();//ms5611 init

	get_DHT_Temp_Hum(&temperature, &humidity);
	sprintf(buffer, "Temperature:%.1f Humidity:%.1f ",temperature,humidity);
	Uart_SendString(buffer, pc_uart);


	uint32_t pressure = getPressure();
	sprintf(buffer, "Pressure %X %lu\r\n",(unsigned int) pressure,pressure);
	Uart_SendString(buffer, pc_uart);

	Uart_SendString("Begin 2250 ms timer (htim3)\r\n", pc_uart);
	HAL_TIM_Base_Start_IT(&htim3);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		//1. refresh watchdog timer - the ultimate restart failsafe if something else does not work
		HAL_IWDG_Refresh(&hiwdg);

		//2. has a shutdown been scheduled
		checkShutdown();//actually executes the shutdown if scheduled an the timer has expired.

		//3. If the time is ok, save settings to SD (if there are new settings, and check for restart hour.
		if(getTimeState() == TIMESTATE_OK_NITZ || getTimeState() == TIMESTATE_OK_SERVER){
			saveSettingsToSD();

			thisHour = getHour();
			if(thisHour == getSettings()->RSH && thisHour!=lastHour && lastHour != H_TEST){//it is the restart hour, and the first time we have seen it.
				sprintf(buffer, "Restart hour %i reached. Restarting... \r\n\r\n",getSettings()->RSH);
				Uart_SendString(buffer, pc_uart);

				shutdown();

			}

			lastHour = thisHour;
		}

		//4. Do we have a new measurement to deal with? If the time is ok, add time to it and add it to the measurement list ready for sending
		if(newMeasurement ){
			if(getTimeState() == TIMESTATE_OK_NITZ || getTimeState() == TIMESTATE_OK_SERVER){
				currentMeasurement.index = powerOnIndex++;
				getEpochTime(&currentMeasurement.unix_time);
				currentMeasurement.pressure = getPressure();

				uint8_t res;
				res = get_DHT_Temp_Hum(&temperature, &humidity);
				if(!res){
					Uart_SendString("\r\nTemperature and Humidity not updated. DHT22 Not Responding", pc_uart);
				}
				currentMeasurement.temperature = temperature;
				currentMeasurement.humidity = humidity;


				currentMeasurement.csq = 199;

				addMeasurement(&currentMeasurement);
				newMeasurement = 0;

				Uart_SendString("\r\n*  *  *  *  *\r\nNew FFWX measurement recorded at: ", pc_uart);
				getTimeChar(buffer);
				Uart_SendString(buffer, pc_uart);
				Uart_SendString("\r\n", pc_uart);
				measurementToSentance(&currentMeasurement, buffer);
				Uart_SendString(buffer, pc_uart);
				Log(buffer,1);
			}
			else{
				Uart_SendString("\r\n*  *  *  *  *\r\nNew FFWX measurement discarded - time not set\r\n ", pc_uart);
				newMeasurement = 0;
//				currentMeasurement.unix_time.ms = 0;
//				currentMeasurement.unix_time.seconds = 0;
			}


		}

		//5. Write data to the SD card.
		//Only write to the SD card if it less than 3ms since the last external interrupt cased by the anemometer,
		//or if it is less than 3ms since a 2.25s timer interrupt with 0 measurements.
		//This minimizes the chance of a external interrupt from the anemometer in the middle of SD card writing.
		if(timer10_ms_get() <= 3){
			checkWriteLogBuffer();
			checkSDError();
		}

		//6. Manage sending the sim7600 state machine, if the power is ok, then manage commands (and send measurements)
		if(sim7600_powerProcessor()){
			sim7600_commandProcessor();
		}

		//7. Process data from U2, (and send it to U1)
		while (IsDataAvailable(pc_uart)){
			int data = Uart_read(pc_uart);
			pc_uart_processChar(data);
			Uart_write(data, device_uart);
		}

		//8. Process data from U1, (and send it to U2)
		while (IsDataAvailable(device_uart)){
			int data = Uart_read(device_uart);
			sim7600_processChar(data);
			Uart_write(data, pc_uart);
		}



	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 144;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV25;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 125-1;
  hrtc.Init.SynchPrediv = 7999-1;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_SATURDAY;
  sDate.Month = RTC_MONTH_OCTOBER;
  sDate.Date = 24;
  sDate.Year = 20;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SDIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDIO_SD_Init(void)
{

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */

  /* USER CODE END SDIO_Init 1 */
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 2;
  /* USER CODE BEGIN SDIO_Init 2 */

  /* USER CODE END SDIO_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 60000-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 60000-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 2250-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 60000-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 60000-1;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 4294967295;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief TIM9 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM9_Init(void)
{

  /* USER CODE BEGIN TIM9_Init 0 */

  /* USER CODE END TIM9_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};

  /* USER CODE BEGIN TIM9_Init 1 */

  /* USER CODE END TIM9_Init 1 */
  htim9.Instance = TIM9;
  htim9.Init.Prescaler = 60-1;
  htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim9.Init.Period = 65535;
  htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM9_Init 2 */

  /* USER CODE END TIM9_Init 2 */

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 60000-1;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 65535;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIO_OUT_4_1EN_GPIO_Port, GPIO_OUT_4_1EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_OUT_DTR_Pin|GPIO_OUT_RESET_Pin|GPIO_OUT_PWRKEY_Pin|GPIO_OUT_LED_Pin
                          |GPIO_OUT_U_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : GPIO_OUT_4_1EN_Pin */
  GPIO_InitStruct.Pin = GPIO_OUT_4_1EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIO_OUT_4_1EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO_IN_POWERSTAT_Pin GPIO_IN_RI_Pin GPIO_IN_DCD_Pin GPIO_TH_DATA_Pin */
  GPIO_InitStruct.Pin = GPIO_IN_POWERSTAT_Pin|GPIO_IN_RI_Pin|GPIO_IN_DCD_Pin|GPIO_TH_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO_OUT_DTR_Pin GPIO_OUT_RESET_Pin GPIO_OUT_PWRKEY_Pin GPIO_OUT_LED_Pin
                           GPIO_OUT_U_CS_Pin */
  GPIO_InitStruct.Pin = GPIO_OUT_DTR_Pin|GPIO_OUT_RESET_Pin|GPIO_OUT_PWRKEY_Pin|GPIO_OUT_LED_Pin
                          |GPIO_OUT_U_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_CD_Pin */
  GPIO_InitStruct.Pin = SD_CD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SD_CD_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO_IN_WINDCOUNT_Pin */
  GPIO_InitStruct.Pin = GPIO_IN_WINDCOUNT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIO_IN_WINDCOUNT_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */

RTC_HandleTypeDef * getHRTC(void){
	return &hrtc;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
//	unsigned int timems;
//	char buf[30];
//	timems = (unsigned int)timer10_ms_get();
	if(GPIO_Pin == GPIO_IN_WINDCOUNT_Pin){//this is the windspeed external interrupt occurring on a rising edge; at 200mph = at least 11.25 ms apart
//		sprintf(buf, "XTime %u \r\n", timems);
//		Uart_SendString(buf, pc_uart);

//		delay_us(5);//debounce for 5us

		if(HAL_GPIO_ReadPin(GPIO_IN_WINDCOUNT_GPIO_Port, GPIO_IN_WINDCOUNT_Pin)){
//			Uart_SendString("O\r\n", pc_uart);
			windCount++;
		}
	}
	timer10_ms_restart();
}

// Callback: timer has rolled over
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	// Check which version of the timer triggered this callback and toggle LED
	if (htim == &htim3 )
	{
//		HAL_GPIO_TogglePin(GPIO_OUT_LED_GPIO_Port, GPIO_OUT_LED_Pin);

		uint16_t p = windCount;
		windCount = 0;

//	    mph = p * (2.25/PERIOD);

		if(p == 0){
			timer10_ms_restart();
		}

		mph = p;
	    direction = 0.087890625 * adcValues[0];

//	    sprintf(buffer, "$FFWX,0,%i,%.0f\r\n",mph,direction);
//	   	Uart_SendString(buffer, pc_uart);

//	   	getEpochTime();

	    vectorX += mph * sin(deg2rad * direction);
	    vectorY += mph * cos(deg2rad * direction);

	    if(mph < minSpeed){
	        minSpeed = mph;
	    }
	    if(mph > maxSpeed){
	        maxSpeed = mph;
	    }

	    batSum += adcValues[1]*getSettings()->BCL;

	    avgCount++;

	    avgNum = getSettings()->AVN;

	    if(avgCount >= avgNum){

	        vectorX = vectorX/avgNum;
	        vectorY = vectorY/avgNum;

	        avgSpeed = sqrt(vectorX * vectorX + vectorY * vectorY);
	        avgDirection =  rad2deg * atan2(vectorX, vectorY);

	        if(avgDirection < 0){
	            avgDirection += 360;
	        }

	        if(avgSpeed < 0.001){
	            avgDirection = direction;
	        }


	        currentMeasurement.direction = avgDirection;
	        currentMeasurement.mph = avgSpeed;
	        currentMeasurement.mphMax = maxSpeed;
	        currentMeasurement.mphMin = minSpeed;
	        currentMeasurement.battery = batSum/avgNum;

	        newMeasurement = 1;

	        HAL_GPIO_WritePin(GPIO_OUT_LED_GPIO_Port, GPIO_OUT_LED_Pin,GPIO_PIN_SET);


	        avgCount = 0;
	        vectorX = 0;
	        vectorY = 0;
	        minSpeed = 1000;
	        maxSpeed = 0;
	        batSum = 0;

	    }

	    if(avgCount == 1){
	    	HAL_GPIO_WritePin(GPIO_OUT_LED_GPIO_Port, GPIO_OUT_LED_Pin,GPIO_PIN_RESET);
	    }
	}
}

int8_t user_spi_read(uint8_t reg_addr, uint8_t *reg_data, uint16_t len){

	HAL_GPIO_WritePin(GPIO_OUT_U_CS_GPIO_Port, GPIO_OUT_U_CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(&hspi1, &reg_addr , 1,1);
	HAL_SPI_Receive(&hspi1, reg_data, len,1);

	HAL_GPIO_WritePin(GPIO_OUT_U_CS_GPIO_Port, GPIO_OUT_U_CS_Pin, GPIO_PIN_SET);

	int8_t rslt = 0;				// fake goodness


	return(rslt);

}

int8_t user_spi_write(uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {

	HAL_GPIO_WritePin(GPIO_OUT_U_CS_GPIO_Port, GPIO_OUT_U_CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(&hspi1, &reg_addr , 1,1);
	if(len > 0){
		HAL_SPI_Transmit(&hspi1, reg_data, len,1);
	}


	HAL_GPIO_WritePin(GPIO_OUT_U_CS_GPIO_Port, GPIO_OUT_U_CS_Pin, GPIO_PIN_SET);

	int8_t rslt = 0; // fake goodness


	return(rslt);

}

void delay_us (uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim9,0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(&htim9) < us);  // wait for the counter to reach the us input in the parameter
}

void timer2_ms_restart(void){
	HAL_TIM_Base_Stop(&htim2);
	__HAL_TIM_SET_COUNTER(&htim2,0);  // set the counter value a 0
	HAL_TIM_Base_Start(&htim2);
}

uint32_t timer2_ms_get(void){
	return __HAL_TIM_GET_COUNTER(&htim2);
}

void timer4_ms_restart(void){
	HAL_TIM_Base_Stop(&htim4);
	__HAL_TIM_SET_COUNTER(&htim4,0);  // set the counter value a 0
	HAL_TIM_Base_Start(&htim4);
}

uint32_t timer4_ms_get(void){
	return __HAL_TIM_GET_COUNTER(&htim4);
}

void timer5_ms_restart(void){
	HAL_TIM_Base_Stop(&htim5);
	__HAL_TIM_SET_COUNTER(&htim5,0);  // set the counter value a 0
	HAL_TIM_Base_Start(&htim5);
}

uint32_t timer5_ms_get(void){
	return __HAL_TIM_GET_COUNTER(&htim5);
}

void timer10_ms_restart(void){
	HAL_TIM_Base_Stop(&htim10);
	__HAL_TIM_SET_COUNTER(&htim10,0);  // set the counter value a 0
	HAL_TIM_Base_Start(&htim10);
}

uint32_t timer10_ms_get(void){
	return __HAL_TIM_GET_COUNTER(&htim10);
}




/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
