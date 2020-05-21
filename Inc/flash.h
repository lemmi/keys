#ifndef FLASH_H
#define FLASH_H

#include "stm32f0xx_hal.h"

uint16_t          flash_layout_load();
HAL_StatusTypeDef flash_layout_append(uint16_t);

#endif
