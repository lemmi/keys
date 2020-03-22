/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file		   : main.c
 * @brief		   : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *						  opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "usbd_customhid.h"
#include "layouts.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */

uint16_t USB_EVENT;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_DMA_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void GPIO_AS_INPUT();
static void GPIO_AS_INT();
typedef enum {
	RUNNING,
	SUSPEND_ENTER,
	SUSPEND,
	SUSPEND_EXIT,
	REMOTE_WAKE,
} State_t;

typedef const uint16_t Hand_t[ROWS];

typedef struct {
	uint8_t report[8];
	uint8_t history[HISTORY_SIZE][ROWS];
	uint8_t hist_idx;
	Layout_t *layout;
	Hand_t *hand;
} Keys_t;

static void k_clear(Keys_t *k) {
	memset(k, 0, sizeof(k->report) + sizeof(k->history) + sizeof(k->hist_idx));
}

static uint16_t k_all_rows(const Keys_t *k) {
	return (*k->hand)[0] | (*k->hand)[1] | (*k->hand)[2] | (*k->hand)[3] | (*k->hand)[4] | (*k->hand)[5];
}

static void k_scan(Keys_t *k) {
	k->hist_idx = (k->hist_idx + 1) % HISTORY_SIZE;
	get_rows(k->history[k->hist_idx], *k->hand);
}

static void k_merge_history(const Keys_t *k, uint8_t merged[ROWS]) {
	size_t i, r;
	for (i = 0; i < HISTORY_SIZE; i++) {
		for (r = 0; r < ROWS; r++) {
			merged[r] |= k->history[i][r];
		}
	}
}

static void k_report(Keys_t *k) {
	uint8_t merged[ROWS] = {0};
	uint8_t buttons[NSWITCH] = {0};
	uint8_t pressed = 0;

	k->report[0] = 0;
	memset(&k->report[2], 0, 6);

	k_merge_history(k, merged);

	pressed = lyt_get_pressed(k->layout, buttons, &k->report[0], merged);
	if (pressed > 6) {
		pressed = 6;
	}
	memcpy(&k->report[2], buttons, pressed);
	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, k->report, sizeof(k->report));
}

void HAL_SuspendTick() {
	SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
}

void HAL_ResumeTick() {
	SysTick->CTRL |= (SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
}
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
	MX_USART1_UART_Init();
	MX_DMA_Init();
	MX_USB_DEVICE_Init();
	/* USER CODE BEGIN 2 */

	GPIO_AS_INPUT();
	// we have no need for systick. disable source and mask interrupt
	HAL_SuspendTick();


	Keys_t k;
	k_clear(&k);
	lyt_select_layout(&k.layout, &k.hand);

	State_t state = RUNNING;

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		USB_EVENT = 0;

		HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

loop_no_sleep:
		switch (state) {
			case RUNNING:
				if (USB_EVENT & USB_ISTR_SUSP) {
					state = SUSPEND_ENTER;
					goto loop_no_sleep;
				}

				if (!(USB_EVENT & USB_ISTR_SOF)) {
					break;
				}

				k_scan(&k);
				k_report(&k);
				break;
			case SUSPEND_ENTER:
				k_clear(&k);
				GPIO_AS_INT();
				HAL_GPIO_WritePin(GPIOB, k_all_rows(&k), GPIO_PIN_SET);
				state = SUSPEND;
				break;
			case SUSPEND:
				if (USB_EVENT & USB_ISTR_WKUP) {
					state = SUSPEND_EXIT;
					goto loop_no_sleep;
				}
				if ((GPIOA->IDR & 0xFF) != 0) {
					state = REMOTE_WAKE;
					goto loop_no_sleep;
				}
				break;
			case REMOTE_WAKE:
				// we need the systick to measure 10ms for the wakeup
				HAL_ResumeTick();
				HAL_PCD_ActivateRemoteWakeup(hUsbDeviceFS.pData);
				HAL_Delay(10);
				HAL_PCD_DeActivateRemoteWakeup(hUsbDeviceFS.pData);
				state=SUSPEND_EXIT;
				HAL_SuspendTick();
				goto loop_no_sleep;
			case SUSPEND_EXIT:
				HAL_GPIO_WritePin(GPIOB, k_all_rows(&k), GPIO_PIN_RESET);
				GPIO_AS_INPUT();
				state = RUNNING;
				break;
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
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
	RCC_CRSInitTypeDef RCC_CRSInitStruct = {0};

	/** Initializes the CPU, AHB and APB busses clocks 
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
	RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks 
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
		|RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
	{
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART1;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
	PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;

	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}
	/** Enable the SYSCFG APB clock 
	*/
	__HAL_RCC_CRS_CLK_ENABLE();
	/** Configures CRS 
	*/
	RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
	RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;
	RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
	RCC_CRSInitStruct.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000,1000);
	RCC_CRSInitStruct.ErrorLimitValue = 34;
	RCC_CRSInitStruct.HSI48CalibrationValue = 32;

	HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
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
	huart1.Init.BaudRate = 38400;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_HalfDuplex_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/** 
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) 
{

	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Channel2_3_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

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
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13 
			|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_4|GPIO_PIN_5 
			|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

	/*Configure GPIO pins : PC13 PC14 PC15 */
	GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PF0 PF1 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	/*Configure GPIO pins : PA0 PA1 PA2 PA3 
	  PA4 PA5 PA6 PA7 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
		|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB0 PB1 PB2 PB3 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PB10 PB11 PB12 PB13 
	  PB14 PB15 PB4 PB5 
	  PB6 PB7 PB8 PB9 */
	GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13 
		|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_4|GPIO_PIN_5 
		|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PA8 PA10 PA15 */
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_10|GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

	HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 4 */

static void GPIO_AS_INT() {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/*Configure GPIO pins : PA0 PA1 PA2 PA3
	  PA4 PA5 PA6 PA7 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
		|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
static void GPIO_AS_INPUT() {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/*Configure GPIO pins : PA0 PA1 PA2 PA3
	  PA4 PA5 PA6 PA7 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
		|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
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
