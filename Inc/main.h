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
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "stm32f0xx_hal.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_crs.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_gpio.h"

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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define GPIO_LEFT_0_Pin LL_GPIO_PIN_10
#define GPIO_LEFT_0_GPIO_Port GPIOB
#define GPIO_LEFT_1_Pin LL_GPIO_PIN_11
#define GPIO_LEFT_1_GPIO_Port GPIOB
#define GPIO_LEFT_2_Pin LL_GPIO_PIN_12
#define GPIO_LEFT_2_GPIO_Port GPIOB
#define GPIO_LEFT_3_Pin LL_GPIO_PIN_13
#define GPIO_LEFT_3_GPIO_Port GPIOB
#define GPIO_LEFT_4_Pin LL_GPIO_PIN_14
#define GPIO_LEFT_4_GPIO_Port GPIOB
#define GPIO_LEFT_5_Pin LL_GPIO_PIN_15
#define GPIO_LEFT_5_GPIO_Port GPIOB
#define GPIO_RIGHT_5_Pin LL_GPIO_PIN_4
#define GPIO_RIGHT_5_GPIO_Port GPIOB
#define GPIO_RIGHT_4_Pin LL_GPIO_PIN_5
#define GPIO_RIGHT_4_GPIO_Port GPIOB
#define GPIO_RIGHT_3_Pin LL_GPIO_PIN_6
#define GPIO_RIGHT_3_GPIO_Port GPIOB
#define GPIO_RIGHT_2_Pin LL_GPIO_PIN_7
#define GPIO_RIGHT_2_GPIO_Port GPIOB
#define GPIO_RIGHT_1_Pin LL_GPIO_PIN_8
#define GPIO_RIGHT_1_GPIO_Port GPIOB
#define GPIO_RIGHT_0_Pin LL_GPIO_PIN_9
#define GPIO_RIGHT_0_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
