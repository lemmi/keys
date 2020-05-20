#include "main.h"

#include "usb_device.h"
#include "usbd_customhid.h"

#include "app.h"
#include "layouts.h"
#include "bits.h"

#define HISTORY_SIZE 3

__IO int complete;

extern UART_HandleTypeDef huart1;
extern USBD_HandleTypeDef hUsbDeviceFS;
extern TIM_HandleTypeDef htim14;

static void GPIO_AS_INPUT();
static void GPIO_AS_INT();
static uint8_t SOFCallback(USBD_HandleTypeDef *pdev);

typedef enum {
	NONE,
	SUSPEND,
	REMOTE_WAKE,
	SCAN,
} Reason_t;

__IO Reason_t REASON;

typedef const uint16_t Hand_t[ROWS];

typedef struct {
	uint8_t history[HISTORY_SIZE][ROWS];
	uint8_t history_last[ROWS];
	uint8_t hist_idx;
	const Layout_t *layout;
} Keys_t __attribute__ ((aligned (4)));

Keys_t k = {0};

static void k_clear(Keys_t *k) {
	memset(k, 0, sizeof(k->history) + sizeof(k->hist_idx));
}

static void k_scan(Keys_t *k) {
	k->hist_idx = (k->hist_idx + 1) % HISTORY_SIZE;
	get_rows(k->history[k->hist_idx]);
}

static void k_merge_history(const Keys_t *k, uint8_t merged[restrict ROWS]) {
	size_t i, r;
	for (i = 0; i < HISTORY_SIZE; i++) {
		for (r = 0; r < ROWS; r++) {
			merged[r] |= k->history[i][r];
		}
	}
}

static void k_report(Keys_t *k) {
	uint8_t merged[ROWS] = {0};
	int cmp;

	k_merge_history(k, merged);

	cmp = memcmp(k->history_last, merged, ROWS);
	memcpy(k->history_last, merged, ROWS);
	if (cmp == 0) {
		return;
	}

	if(((USBD_CUSTOM_HID_HandleTypeDef *)hUsbDeviceFS.pClassData)->Protocol == 0) {
		// handle boot protocol
		uint8_t report[8] = {0};
		lyt_report_boot(k->layout, report, merged);
		USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, sizeof(report));
	} else {
		// handle report protocol
		Bits_t report_bits = {0};
		lyt_report_bits(k->layout, report_bits, merged);
		USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *) report_bits, 29);
		//	if (complete) {
		//		complete = 0;
		//		HAL_UART_Transmit_DMA(&huart1, (uint8_t *)report_bits, 29);
		//	}
	}
}

void HAL_SuspendTick() {
	SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
}

void HAL_ResumeTick() {
	SysTick->CTRL |= (SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
}

void app() {
	GPIO_AS_INPUT();

	k_clear(&k);
	lyt_select_layout(&k.layout);

	// we have no need for systick. disable source and mask interrupt
	HAL_SuspendTick();

	hUsbDeviceFS.pClass->SOF=SOFCallback;

	complete = 1;
	while (1) {
		HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	}
}

static uint8_t SOFCallback(USBD_HandleTypeDef *pdev) {
	htim14.Instance->ARR = 4; // wait 500us to scan
	REASON = SCAN;
	HAL_TIM_Base_Start_IT(&htim14);
	return 0;
}
static void SuspendCallback() {
	k_clear(&k);
	htim14.Instance->ARR = 999; // wait 100ms
	REASON = SUSPEND;
	HAL_TIM_Base_Start_IT(&htim14);
}
static void ResumeCallback() {
	LL_GPIO_ResetOutputPin(GPIOB, lyt_all_rows);
	GPIO_AS_INPUT();
}
void USB_Callback() {
	/*
	uint16_t dbg_rows = 0;

	dbg_rows |= (USB->ISTR & USB_ISTR_ESOF) ? (*k.hand)[0] : (uint16_t) 0;
	dbg_rows |= (USB->ISTR & USB_ISTR_SOF)  ? (*k.hand)[1] : (uint16_t) 0;
	dbg_rows |= (USB->ISTR & USB_ISTR_CTR)  ? (*k.hand)[2] : (uint16_t) 0;
	dbg_rows |= (USB->ISTR & USB_ISTR_SUSP) ? (*k.hand)[3] : (uint16_t) 0;
	dbg_rows |= (USB->ISTR & USB_ISTR_WKUP) ? (*k.hand)[4] : (uint16_t) 0;
	dbg_rows |= (USB->ISTR & USB_ISTR_ERR)  ? (*k.hand)[5] : (uint16_t) 0;
	LL_GPIO_SetOutputPin(GPIOB, dbg_rows);
	*/

	if (USB->ISTR & USB_ISTR_SUSP) {
		SuspendCallback();
	}
	if (USB->ISTR & USB_ISTR_WKUP) {
		ResumeCallback();
	}
	//LL_GPIO_ResetOutputPin(GPIOB, k_all_rows(&k));
}
static void Remote_Wake() {
	if ((GPIOA->IDR & 0xFF) == 0) {
		// there is no button pressed that we are interested in
		return;
	}
	HAL_PCD_ActivateRemoteWakeup(hUsbDeviceFS.pData);
	htim14.Instance->ARR = 99; // 10ms
	REASON = REMOTE_WAKE;
	HAL_TIM_Base_Start_IT(&htim14);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	HAL_TIM_Base_Stop_IT(&htim14);
	if (REASON == SUSPEND) {
		GPIO_AS_INT();
		LL_GPIO_SetOutputPin(GPIOB, lyt_all_rows);
	}
	if (REASON == REMOTE_WAKE) {
		HAL_PCD_DeActivateRemoteWakeup(hUsbDeviceFS.pData);
	}
	if (REASON == SCAN) {
		k_scan(&k);
		k_report(&k);
		/*
		if (complete) {
			complete = 0;
			HAL_UART_Transmit_DMA(&huart1, (uint8_t *)k.report, sizeof(k.report));
		}
		*/
	}
	REASON = NONE;
}

static void GPIO_AS_INT() {
	LL_EXTI_InitTypeDef EXTI_InitStruct = {0};

	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE0);
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE1);
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE2);
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE3);
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE4);
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE5);
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE6);
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE7);

	EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_0 | LL_EXTI_LINE_1 | LL_EXTI_LINE_2 |
		LL_EXTI_LINE_3 | LL_EXTI_LINE_4 | LL_EXTI_LINE_5 |
		LL_EXTI_LINE_6 | LL_EXTI_LINE_7;
	EXTI_InitStruct.LineCommand = ENABLE;
	EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
	LL_EXTI_Init(&EXTI_InitStruct);

	NVIC_SetPriority(EXTI0_1_IRQn, 0);
	NVIC_EnableIRQ(EXTI0_1_IRQn);
	NVIC_SetPriority(EXTI2_3_IRQn, 0);
	NVIC_EnableIRQ(EXTI2_3_IRQn);
	NVIC_SetPriority(EXTI4_15_IRQn, 0);
	NVIC_EnableIRQ(EXTI4_15_IRQn);
}

static void GPIO_AS_INPUT() {
	LL_EXTI_InitTypeDef EXTI_InitStruct = {0};

	EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_0 | LL_EXTI_LINE_1 | LL_EXTI_LINE_2 |
		LL_EXTI_LINE_3 | LL_EXTI_LINE_4 | LL_EXTI_LINE_5 |
		LL_EXTI_LINE_6 | LL_EXTI_LINE_7;
	EXTI_InitStruct.LineCommand = DISABLE;
	EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_NONE;
	LL_EXTI_Init(&EXTI_InitStruct);

	NVIC_DisableIRQ(EXTI0_1_IRQn);
	NVIC_DisableIRQ(EXTI2_3_IRQn);
	NVIC_DisableIRQ(EXTI4_15_IRQn);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) { 
	complete = 1;
}

void EXTI0_1_IRQHandler(void) {
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
	}
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_1) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_1);
	}
	Remote_Wake();
}

void EXTI2_3_IRQHandler(void) {
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_2) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);
	}
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_3) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_3);
	}
	Remote_Wake();
}

void EXTI4_15_IRQHandler(void) {
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_4) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_4);
	}
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_5) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_5);
	}
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_6) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_6);
	}
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_7) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_7);
	}
	Remote_Wake();
}
