#include "main.h"

#include "usb_device.h"
#include "usbd_customhid.h"

#include "app.h"
#include "layouts.h"

#define HISTORY_SIZE 3

__IO uint16_t USB_EVENT;
__IO int complete;

extern UART_HandleTypeDef huart1;
extern USBD_HandleTypeDef hUsbDeviceFS;
extern TIM_HandleTypeDef htim14;

static void GPIO_AS_INPUT();
static void GPIO_AS_INT();

typedef enum {
	NONE,
	SUSPEND,
	REMOTE_WAKE,
} Reason_t;

__IO Reason_t REASON;

typedef const uint16_t Hand_t[ROWS];

typedef struct {
	uint8_t report[8];
	uint8_t history[HISTORY_SIZE][ROWS];
	uint8_t hist_idx;
	Layout_t *layout;
	Hand_t *hand;
} Keys_t;

Keys_t k;

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

void app() {
	GPIO_AS_INPUT();

	k_clear(&k);
	lyt_select_layout(&k.layout, &k.hand);

	// we have no need for systick. disable source and mask interrupt
	HAL_SuspendTick();

	complete = 1;
	while(1) {
		HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	}
}

static void SOFCallback() {
	k_scan(&k);
	k_report(&k);
	if (complete) {
		complete = 0;
		HAL_UART_Transmit_DMA(&huart1, (uint8_t *) k.report, sizeof(k.report));
	}
}
static void SuspendCallback() {
	k_clear(&k);
	REASON = SUSPEND;
	htim14.Instance->ARR=1000;
	HAL_TIM_Base_Start_IT(&htim14);
}
static void ResumeCallback() {
	LL_GPIO_ResetOutputPin(GPIOB, k_all_rows(&k));
	GPIO_AS_INPUT();
}
void USB_Callback() {
	USB_EVENT |= USB->ISTR;

	if(USB->ISTR & USB_ISTR_SOF){
		SOFCallback();
	}
	if(USB->ISTR & USB_ISTR_SUSP){
		SuspendCallback();
	}
	if(USB->ISTR & USB_ISTR_SOF){
		ResumeCallback();
	}
}
static void Remote_Wake() {
	if ((GPIOA->IDR & 0xFF) == 0) {
		// there is no button pressed that we are interested in
		return;
	}
	HAL_PCD_ActivateRemoteWakeup(hUsbDeviceFS.pData);
	REASON = REMOTE_WAKE;
	htim14.Instance->ARR=10;
	HAL_TIM_Base_Start_IT(&htim14);
}
void Timer_Callback() {
	HAL_TIM_Base_Stop_IT(&htim14);
	if (REASON == SUSPEND) {
		GPIO_AS_INT();
		LL_GPIO_SetOutputPin(GPIOB, k_all_rows(&k));
	}
	if (REASON == REMOTE_WAKE) {
		HAL_PCD_DeActivateRemoteWakeup(hUsbDeviceFS.pData);
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

	EXTI_InitStruct.Line_0_31 =
		LL_EXTI_LINE_0 |
		LL_EXTI_LINE_1 |
		LL_EXTI_LINE_2 |
		LL_EXTI_LINE_3 |
		LL_EXTI_LINE_4 |
		LL_EXTI_LINE_5 |
		LL_EXTI_LINE_6 |
		LL_EXTI_LINE_7;
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

	EXTI_InitStruct.Line_0_31 =
		LL_EXTI_LINE_0 |
		LL_EXTI_LINE_1 |
		LL_EXTI_LINE_2 |
		LL_EXTI_LINE_3 |
		LL_EXTI_LINE_4 |
		LL_EXTI_LINE_5 |
		LL_EXTI_LINE_6 |
		LL_EXTI_LINE_7;
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

void EXTI0_1_IRQHandler(void)
{
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
	}
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_1) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_1);
	}
	Remote_Wake();
}

void EXTI2_3_IRQHandler(void)
{
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_2) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);
	}
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_3) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_3);
	}
	Remote_Wake();
}

void EXTI4_15_IRQHandler(void)
{
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

