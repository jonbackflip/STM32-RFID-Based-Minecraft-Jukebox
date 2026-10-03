/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "SSD1306Driver.h"
#include "MFRC522_STM32.h"
#include "WavPlayer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define TAU 6.28318530717958647692

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

I2S_HandleTypeDef hi2s1;
DMA_HandleTypeDef hdma_spi1_tx;

SD_HandleTypeDef hsd;
DMA_HandleTypeDef hdma_sdio;

SPI_HandleTypeDef hspi5;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

// ---- OLED redraw throttle (set by TIM2 ISR, checked once per main loop) --
volatile uint8_t oledNeedsRedraw = 0;

// ---- RFID polling state (unchanged from before) --------------------------
#define RFID_POLL_INTERVAL_MS 100
typedef enum { RFID_WAIT_CARD, RFID_CARD_PRESENT } RfidState_t;
static RfidState_t rfidState = RFID_WAIT_CARD;
static uint32_t rfidLastPoll = 0;

#define RFID_DETECT_CONFIRM_COUNT   3
#define RFID_REMOVE_CONFIRM_COUNT   5

static uint8_t rfidGoodStreak = 0;
static uint8_t rfidBadStreak  = 0;

// ---- UI polling (volume pot, pause button, headphone jack) --------------
#define UI_POLL_INTERVAL_MS 20
static uint32_t uiLastPoll = 0;
static volatile uint8_t volumePercent = 100;
static uint8_t headphoneConnected = 0;
static uint8_t pauseButtonWasDown = 0;

// ---- Song table: RFID UID -> WAV file on the SD card ---------------------
// Add one entry per tag. Filenames are LFN paths
typedef struct {
    uint8_t uid[4];
    const char *filePath;
} SongEntry_t;

static const SongEntry_t songTable[] = {
    { {0x81, 0xA0, 0xC9, 0x66}, "Cat.WAV" },  // WHITE CARD
	{ {0xD0, 0x8A, 0x78, 0x5C}, "Blocks.WAV" },
	{ {0xC7, 0x6D, 0x8D, 0x64}, "Chirp.WAV"  },
	{ {0xD0, 0x3B, 0x70, 0x5C}, "Far.WAV"  },
	{ {0x21, 0x56, 0x62, 0x64}, "Mall.WAV"  },
	{ {0xD0, 0xE6, 0x7C, 0x5C}, "Mellohi.WAV" },
	{ {0x21, 0x73, 0x3A, 0x64}, "Stal.WAV" },
	{ {0xE0, 0x5B, 0x7D, 0x5C}, "Strad.WAV" },
	{ {0xD0, 0xE6, 0x6A, 0x5C}, "Ward.WAV" },
	{ {0x00, 0x17, 0x1A, 0x56}, "Wait.WAV" },
	{ {0x71, 0x9B, 0x3E, 0x6E}, "Otherside.WAV" },
	{ {0xD0, 0x92, 0xA2, 0x5C}, "Relic.WAV" },
};
#define SONG_TABLE_COUNT (sizeof(songTable) / sizeof(songTable[0]))

static const SongEntry_t *find_song_by_uid(const uint8_t *uid)
{
    for (uint32_t i = 0; i < SONG_TABLE_COUNT; i++)
        if (memcmp(songTable[i].uid, uid, 4) == 0)
            return &songTable[i];
    return NULL;
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2S1_Init(void);
static void MX_SDIO_SD_Init(void);
static void MX_SPI5_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int fd, unsigned char *buf, int len) {
  if (fd == 1 || fd == 2) {                     // stdout or stderr ?
    HAL_UART_Transmit(&huart2, buf, len, HAL_MAX_DELAY);  // Print to the UART
  }
  return len;
}

uint8_t uid[4];
MFRC522_t rfID = {&hspi5, CS_GPIO_Port, CS_Pin, RESET_GPIO_Port, RESET_Pin};
WavPlayer_t player;

static void RFID_Poll(MFRC522_t *dev, WavPlayer_t *wp);
static void UI_Poll(WavPlayer_t *wp);
static void OLED_Redraw(WavPlayer_t *wp);

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
  MX_I2S1_Init();
  MX_SDIO_SD_Init();
  MX_SPI5_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_FATFS_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
  __HAL_TIM_ENABLE_IT(&htim4, TIM_IT_UPDATE);

  ssd1306_init();

  MFRC522_Init(&rfID);

  WavPlayer_Init(&player);
  WavPlayer_SetVolume(volumePercent);

  FRESULT mountResult = f_mount(&SDFatFS, SDPath, 1); // 1 = mount now
  if (mountResult != FR_OK)
  {
      printf("SD mount failed: FRESULT=%d\r\n", mountResult);
      Error_Handler();
  }
  printf("SD mounted OK\r\n");


  printf("\n\n\n---------------------\nStarting jukebox\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // 1. I2s DMA feed is of HIGHEST PRIORITY
	  WavPlayer_Poll(&player);

	  if (WavPlayer_IsFinished(&player))
	  {
		  WavPlayer_Stop(&player); // record's done -- go quiet until next tag
	  }

	  // 2. UI peripherals -- encoder volume, encoder pause button, headphone jack.
	  // Polled every 20ms
	  UI_Poll(&player);

	  // 3. RFID -- polls every 100ms. Worst-case internal blocking of
	  //    ~50ms in RequestA's timeout/error path fits well inside the
	  //    ~93ms of slack the 4096-frame I2S half-buffer provides.
	  RFID_Poll(&rfID, &player);



	  // 4. OLED -- updated every 250ms by flag set by the TIM2 ISR,
	  //    not redrawn every single pass.
	  if (oledNeedsRedraw)
	  {
		  oledNeedsRedraw = 0;
		  OLED_Redraw(&player);
	  }



    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2S1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S1_Init(void)
{

  /* USER CODE BEGIN I2S1_Init 0 */

  /* USER CODE END I2S1_Init 0 */

  /* USER CODE BEGIN I2S1_Init 1 */

  /* USER CODE END I2S1_Init 1 */
  hi2s1.Instance = SPI1;
  hi2s1.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s1.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s1.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s1.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s1.Init.AudioFreq = I2S_AUDIOFREQ_44K;
  hi2s1.Init.CPOL = I2S_CPOL_LOW;
  hi2s1.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s1.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S1_Init 2 */

  /* USER CODE END I2S1_Init 2 */

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
  hsd.Init.ClockDiv = 0;
  /* USER CODE BEGIN SDIO_Init 2 */
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  /* USER CODE END SDIO_Init 2 */

}

/**
  * @brief SPI5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI5_Init(void)
{

  /* USER CODE BEGIN SPI5_Init 0 */

  /* USER CODE END SPI5_Init 0 */

  /* USER CODE BEGIN SPI5_Init 1 */

  /* USER CODE END SPI5_Init 1 */
  /* SPI5 parameter configuration*/
  hspi5.Instance = SPI5;
  hspi5.Init.Mode = SPI_MODE_MASTER;
  hspi5.Init.Direction = SPI_DIRECTION_2LINES;
  hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi5.Init.NSS = SPI_NSS_SOFT;
  hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi5.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI5_Init 2 */

  /* USER CODE END SPI5_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 10000-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 500-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

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
  htim2.Init.Prescaler = 5000-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 250-1;
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
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 4-1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 12;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 12;
  if (HAL_TIM_Encoder_Init(&htim4, &sConfig) != HAL_OK)
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
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPEAKER_ENABLE_GPIO_Port, SPEAKER_ENABLE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, CS_Pin|RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PAUSE_BTN_Pin */
  GPIO_InitStruct.Pin = PAUSE_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(PAUSE_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPEAKER_ENABLE_Pin */
  GPIO_InitStruct.Pin = SPEAKER_ENABLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPEAKER_ENABLE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : HEADPHONE_JACK_DETECT_Pin SD_DETECT_Pin */
  GPIO_InitStruct.Pin = HEADPHONE_JACK_DETECT_Pin|SD_DETECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_Pin RESET_Pin */
  GPIO_InitStruct.Pin = CS_Pin|RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/*
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        ssd1306_dma_tx_complete();
    }
}
*/

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        oledNeedsRedraw = 1;
    }
    else if (htim->Instance == TIM4)
        {
		// Counter just overflowed (CW click) or underflowed (CCW click).
		// __HAL_TIM_IS_TIM_COUNTING_DOWN reads CR1's DIR bit, which
		// reflects the direction of the count that triggered this
		// update event.
		if (__HAL_TIM_IS_TIM_COUNTING_DOWN(htim))
		{
			volumePercent = (volumePercent >= 5) ? (volumePercent - 5) : 0;
		}
		else
		{
			volumePercent = (volumePercent <= 95) ? (volumePercent + 5) : 100;
		}
		WavPlayer_SetVolume(volumePercent);
        }
}


static void RFID_Poll(MFRC522_t *dev, WavPlayer_t *wp)
{
    uint32_t now = HAL_GetTick();
    if ((now - rfidLastPoll) < RFID_POLL_INTERVAL_MS) return;
    rfidLastPoll = now;

    uint8_t localAtqa[2];
    uint8_t detected = (MFRC522_RequestA(dev, localAtqa) == STATUS_OK);

    if (rfidState == RFID_WAIT_CARD) // Check RFID CARD presence
    {
        if (detected)
        {
            rfidBadStreak = 0;
            rfidGoodStreak++;

            if (rfidGoodStreak >= RFID_DETECT_CONFIRM_COUNT)
            {
                rfidGoodStreak = 0;

                if (MFRC522_ReadUid(dev, uid) == STATUS_OK)
                {
                    USER_LOG("CARD ID:%02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3]);

                    const SongEntry_t *song = find_song_by_uid(uid);
                    if (song)
                    {
                        USER_LOG("Playing %s", song->filePath);
                        if (WavPlayer_Play(wp, song->filePath) != 0)
                        {
                            USER_LOG("Failed to open/parse WAV file");
                        }
                    }
                    else
                    {
                        USER_LOG("Unknown tag -- no song mapped");
                    }
                }
                rfidState = RFID_CARD_PRESENT;
            }
        }
        else
        {
            rfidGoodStreak = 0; // Any miss resets the streak. Must be RFID_DETECT_CONFIRM_COUNT in a row.
        }
    }
    else // RFID_CARD_PRESENT
    {
        if (!detected)
        {
            rfidGoodStreak = 0;
            rfidBadStreak++;

            if (rfidBadStreak >= RFID_REMOVE_CONFIRM_COUNT)
            {
                rfidBadStreak = 0;
                USER_LOG("Card removed");
                WavPlayer_Stop(wp);
                rfidState = RFID_WAIT_CARD;
            }
        }
        else
        {
            rfidBadStreak = 0; // Still there. Reset the removal streak.
        }
    }
}


static void UI_Poll(WavPlayer_t *wp)
{
    uint32_t now = HAL_GetTick();
    if ((now - uiLastPoll) < UI_POLL_INTERVAL_MS) return;
    uiLastPoll = now;

    // Headphone jack detect
    // Active-low switch (grounds the pin when a plug seats).
    uint8_t hpNow = (HAL_GPIO_ReadPin(HEADPHONE_JACK_DETECT_GPIO_Port,
                                       HEADPHONE_JACK_DETECT_Pin) == GPIO_PIN_RESET);
    if (hpNow != headphoneConnected)
    {
        headphoneConnected = hpNow;
        // Mute/enable the speaker amp via its shutdown pin.
        HAL_GPIO_WritePin(SPEAKER_ENABLE_GPIO_Port, SPEAKER_ENABLE_Pin, headphoneConnected ? GPIO_PIN_RESET : GPIO_PIN_SET);
        headphoneConnected ? USER_LOG("Headphones connected") : USER_LOG("Headphones removed");
    }

    // Pause/unpause button
    // Active-low with internal pull-up. The 20ms poll gate
    // above already acts as a debounce for the encoder button.
    uint8_t btnNow = (HAL_GPIO_ReadPin(PAUSE_BTN_GPIO_Port, PAUSE_BTN_Pin) == GPIO_PIN_RESET);
    if (btnNow && !pauseButtonWasDown) // falling edge
    {
        if (wp->state == WAV_PLAYING) WavPlayer_Pause(wp);
        else if (wp->state == WAV_PAUSED) WavPlayer_Resume(wp);
    }
    pauseButtonWasDown = btnNow;
}

static void OLED_Redraw(WavPlayer_t *wp)
{
    ssd1306_clear_buffer();

    if (wp->state == WAV_IDLE || wp->state == WAV_ERROR)
    {
        ssd1306_print(16, 28, "INSERT DISC [][]");
    }
    else
    {
        // Song title
    	char titleLine[40];
    	snprintf(titleLine, sizeof(titleLine), "%s.OGG", wp->title);
    	ssd1306_print(4, 4, titleLine);

        // Progress bar (0-255 fraction scaled to a 100px track)
        uint8_t frac = WavPlayer_GetProgressFraction(wp);
        uint8_t barWidth = (uint8_t)(((uint16_t)frac * 100) / 255);
        ssd1306_draw_rect(10, 22, 104, 10);
        ssd1306_filled_rect(12, 24, barWidth, 6);

        // Elapsed / total time
        char timeStr[20];
        uint32_t elapsed = WavPlayer_GetElapsedSeconds(wp);
        uint32_t total = WavPlayer_GetTotalSeconds(wp);
        snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu  %02lu:%02lu",
                 elapsed / 60, elapsed % 60, total / 60, total % 60);
        ssd1306_print(10, 36, timeStr);

        if (wp->state == WAV_PAUSED) ssd1306_print(10, 48, "PAUSED");
    }

    // Volume bar (always shown)
    ssd1306_draw_rect(10, 56, 104, 6);
    ssd1306_filled_rect(11, 57, (uint8_t)((uint32_t)volumePercent * 102 / 100), 4);

    ssd1306_update(); // non-blocking, DMA-driven
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
