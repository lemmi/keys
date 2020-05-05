#include "main.h"

#include "usb_device.h"
#include "usbd_customhid.h"

#include "app.h"
#include "layouts.h"

__IO uint16_t USB_EVENT;

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

void app() {
	GPIO_AS_INPUT();

	Keys_t k;
	k_clear(&k);
	lyt_select_layout(&k.layout, &k.hand);

	// we have no need for systick. disable source and mask interrupt
	HAL_SuspendTick();

	State_t state = RUNNING;

	while(1) {
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
				// we need the systick to measure 15ms for the wakeup
				HAL_ResumeTick();
				HAL_PCD_ActivateRemoteWakeup(hUsbDeviceFS.pData);
				HAL_Delay(15);
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
	}
}

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
