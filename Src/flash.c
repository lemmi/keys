#include "stm32f0xx.h"

//#define FLASH_SIZE ((__IO uint16_t *)((*(__IO uint16_t *)FLASHSIZE_BASE) *
// 1024))
#define FLASH_LAST_PAGE_BASE (FLASH_BANK1_END + 1UL - FLASH_PAGE_SIZE)
#define FLASH_LAST_PAGE ((__IO uint16_t *) FLASH_LAST_PAGE_BASE)

static uint32_t flash_layout_idx() {
	uint32_t idx;
	for (idx = 0; idx < FLASH_PAGE_SIZE; idx++) {
		if (FLASH_LAST_PAGE[idx] == 0xFFFFU) {
			return idx - 1;
		}
	}
	return 0xFFFFFFFFU;
}

static uint16_t flash_layout_load_idx(uint32_t idx) {
	if (idx < FLASH_PAGE_SIZE) {
		return FLASH_LAST_PAGE[idx];
	}
	return 0xFFFFU;
}

uint16_t flash_layout_load() {
	uint32_t idx = flash_layout_idx();
	return flash_layout_load_idx(idx);
}

static HAL_StatusTypeDef flash_layout_erase_all() {
	HAL_StatusTypeDef      s;
	FLASH_EraseInitTypeDef erase;
	uint32_t               page_error;

	erase.TypeErase   = FLASH_TYPEERASE_PAGES;
	erase.PageAddress = FLASH_LAST_PAGE_BASE;
	erase.NbPages     = 1;

	if ((s = HAL_FLASH_Unlock()) != HAL_OK) {
		return s;
	}
	if ((s = HAL_FLASHEx_Erase(&erase, &page_error)) != HAL_OK) {
		return s;
	}
	if ((s = HAL_FLASH_Lock()) != HAL_OK) {
		return s;
	}

	return HAL_OK;
}

HAL_StatusTypeDef flash_layout_append(uint16_t layout) {
	HAL_StatusTypeDef s;
	uint32_t          idx;

	idx = flash_layout_idx();

	if (flash_layout_load_idx(idx) == layout) {
		return HAL_OK;
	}

	if (idx >= 0xFFFFU) {
		if ((s = flash_layout_erase_all()) != HAL_OK) {
			return s;
		}
		idx = 0;
	} else {
		++idx;
	}

	if ((s = HAL_FLASH_Unlock()) != HAL_OK) {
		return s;
	}
	if ((s = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
	                           (uint32_t) &FLASH_LAST_PAGE[idx],
	                           (uint64_t) layout)) != HAL_OK) {
		return s;
	}
	if ((s = HAL_FLASH_Lock()) != HAL_OK) {
		return s;
	}

	return HAL_OK;
}
