#include "main.h"

#include "usb_device.h"
#include "usbd_customhid.h"

#include "app.h"
#include "bits.h"
#include "layouts.h"

// prefer powers of two to nearby integers.  produces smaller and faster code,
// as this architecture has no division or modulus
#define HISTORY_SIZE 4

__IO int complete;

enum {
	TIMEOUT_HAVE_USB  = 0,
	TIMEOUT_HAVE_UART = 1,
	TIMEOUT_SUSPEND   = 2,
};

__IO uint32_t timeout_ctr = TIMEOUT_HAVE_USB;

extern UART_HandleTypeDef huart1;
extern USBD_HandleTypeDef hUsbDeviceFS;

static void    GPIO_AS_INPUT();
static void    GPIO_AS_INT();
static void    UART_AS_INT();
static void    UART_AS_SINGLE_WIRE();
static void    UART_AS_RX();
static void    UART_AS_TX();
static uint8_t SOFCallback(USBD_HandleTypeDef *pdev);

typedef union {
	uint8_t bytes[ROWS + 4];
	struct {
		uint8_t  rows[ROWS];
		uint32_t chksum;
	};
} Wirefmt_t;

static uint32_t crc(const uint8_t *data, const size_t n) {
	LL_CRC_ResetCRCCalculationUnit(CRC);
	int i = 0;
	// saves 6µS
	for (; i + 3 < n; i += 4) {
		LL_CRC_FeedData32(CRC, *(const uint32_t *) &data[i]);
	}
	for (; i < n; i++) {
		LL_CRC_FeedData8(CRC, data[i]);
	}
	return LL_CRC_ReadData32(CRC);
}

static uint32_t w_crc_calc(const Wirefmt_t *w) { return crc(w->rows, ROWS); }
static void     w_crc_fill(Wirefmt_t *w) { w->chksum = w_crc_calc(w); }
static uint32_t w_crc_check(const Wirefmt_t *w) {
	return w_crc_calc(w) == w->chksum;
}
static void w_clear(Wirefmt_t *w) { memset(w, 0, sizeof(Wirefmt_t)); }

typedef struct {
	uint8_t   history[HISTORY_SIZE][ROWS];
	uint8_t   history_last[ROWS];
	Wirefmt_t MsgBuf;
	uint8_t   hist_idx;
	uint8_t   layout_idx;
} Keys_t __attribute__((aligned(4)));

static Keys_t k = {0};

static void k_clear(Keys_t *k) {
	uint8_t idx = k->layout_idx;
	memset(k, 0, sizeof(*k));
	k->layout_idx = idx;
}

static inline void
vec_or(uint8_t *restrict dst, const uint8_t *restrict src, const size_t n) {
	uint32_t r;
	// manually vectorize this, as the compiler won't do it for us...
	for (r = 0; r + 3 < n; r += 4) {
		*(uint32_t *) &dst[r] |= *(uint32_t *) &src[r];
	}
	for (; r < n; r++) {
		dst[r] |= src[r];
	}
}

static void k_scan(Keys_t *k) {
	k->hist_idx = (k->hist_idx + 1) % HISTORY_SIZE;
	get_rows(k->history[k->hist_idx]);
}

static void k_merge_history(Keys_t *k, uint8_t merged[restrict ROWS]) {
	size_t i;
	// only merge correct serial data into history to get debounced
	if (w_crc_check(&k->MsgBuf)) {
		vec_or(k->history[k->hist_idx], k->MsgBuf.rows, ROWS);
	}
	// do the debounce
	for (i = 0; i < HISTORY_SIZE; i++) {
		vec_or(merged, k->history[i], ROWS);
	}
}

static void k_report(Keys_t *k) {
	USBD_CUSTOM_HID_HandleTypeDef *hhid =
	    (USBD_CUSTOM_HID_HandleTypeDef *) hUsbDeviceFS.pClassData;
	uint8_t         merged[ROWS] = {0};
	const Layout_t *layout;
	int             cmp;

	k_merge_history(k, merged);

	cmp = memcmp(k->history_last, merged, ROWS);
	memcpy(k->history_last, merged, ROWS);
	if (cmp == 0) {
		return;
	}

	if (hhid->Report_buf[0] & 1) {
		// NumLock
		layout = lyt_get_layout_num(k->layout_idx);
	} else {
		layout = lyt_get_layout(k->layout_idx);
	}

	if (hhid->Protocol == 0) {
		// handle boot protocol
		uint8_t report[8] = {0};
		lyt_report_boot(layout, report, merged);
		USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, sizeof(report));
	} else {
		// handle report protocol
		Bits_t report_bits = {0};
		lyt_report_bits(layout, report_bits, merged);
		USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *) report_bits, 29);
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
	k.layout_idx = lyt_select_layout();

	// we have no need for systick. disable source and mask interrupt
	HAL_SuspendTick();

	hUsbDeviceFS.pClass->SOF = SOFCallback;

	complete = 1;

	LL_TIM_ClearFlag_UPDATE(TIM6);
	LL_TIM_EnableIT_UPDATE(TIM6);
	LL_TIM_ClearFlag_UPDATE(TIM7);
	LL_TIM_EnableIT_UPDATE(TIM7);
	LL_TIM_ClearFlag_UPDATE(TIM16);
	LL_TIM_EnableIT_UPDATE(TIM16);

	LL_TIM_ClearFlag_UPDATE(TIM15);
	LL_TIM_EnableIT_UPDATE(TIM15);

	LL_TIM_ClearFlag_UPDATE(TIM17);
	LL_TIM_EnableIT_UPDATE(TIM17);
	LL_TIM_EnableCounter(TIM17);

	while (1) {
		HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	}
}

static uint8_t SOFCallback(USBD_HandleTypeDef *pdev) {
	LL_TIM_DisableCounter(TIM15);
	LL_TIM_DisableCounter(TIM17);
	TIM17->CNT = 0;
	LL_TIM_EnableCounter(TIM17);
	timeout_ctr = TIMEOUT_HAVE_USB;
	LL_TIM_EnableCounter(TIM6); // Wait for 500us

	// this is possibly the best location to signal a ready to receive
	UART_AS_RX();

	HAL_UART_Receive_DMA(&huart1, k.MsgBuf.bytes, sizeof(k.MsgBuf.bytes));

	return 0;
}
void TIM6_DAC_IRQHandler(void) {
	// triggered by SOFCallback
	if (LL_TIM_IsActiveFlag_UPDATE(TIM6)) {
		LL_TIM_ClearFlag_UPDATE(TIM6);
		// at this point we don't want to receive anything from the over half
		// anymore
		HAL_UART_DMAStop(&huart1);
		LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_DOWN);

		k_scan(&k);
		k_report(&k);
	}
}

static void Remote_Wake() {
	UART_AS_SINGLE_WIRE();
	HAL_PCD_ActivateRemoteWakeup(hUsbDeviceFS.pData);
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_UP);
	LL_TIM_EnableCounter(TIM7); // Wait for 10ms
}
void TIM7_IRQHandler(void) {
	// triggered by Remote_Wake
	if (LL_TIM_IsActiveFlag_UPDATE(TIM7)) {
		LL_TIM_ClearFlag_UPDATE(TIM7);
		HAL_PCD_DeActivateRemoteWakeup(hUsbDeviceFS.pData);
		LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_NO);
		UART_AS_INT();
		timeout_ctr = TIMEOUT_HAVE_USB;
		LL_TIM_EnableCounter(TIM17); // rearm timeouts
	}
}

static void SuspendCallback() {
	k_clear(&k);
	LL_TIM_EnableCounter(TIM16); // Wait for 100ms
	UART_AS_INT();
}
void TIM16_IRQHandler(void) {
	// triggered by SuspendCallback
	if (LL_TIM_IsActiveFlag_UPDATE(TIM16)) {
		LL_TIM_ClearFlag_UPDATE(TIM16);
	}
}

void TIM15_IRQHandler(void) {
	if (LL_TIM_IsActiveFlag_UPDATE(TIM15)) {
		LL_TIM_ClearFlag_UPDATE(TIM15);

		GPIO_AS_INT();
		UART_AS_INT();
		timeout_ctr = TIMEOUT_SUSPEND;
		LL_GPIO_SetOutputPin(GPIOB, lyt_all_rows);
	}
}
void TIM17_IRQHandler(void) {
	if (LL_TIM_IsActiveFlag_UPDATE(TIM17)) {
		LL_TIM_ClearFlag_UPDATE(TIM17);
		LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_NO);
		UART_AS_INT();
		timeout_ctr = TIMEOUT_HAVE_UART;
		LL_TIM_EnableCounter(TIM15);
	}
}

static void ResumeCallback() {
	LL_GPIO_ResetOutputPin(GPIOB, lyt_all_rows);
	GPIO_AS_INPUT();
}
void USB_Callback() {
	// uint16_t dbg_rows = 0;
	// dbg_rows |= (USB->ISTR & USB_ISTR_ESOF) ? (*k.hand)[0] : (uint16_t) 0;
	// dbg_rows |= (USB->ISTR & USB_ISTR_SOF)  ? (*k.hand)[1] : (uint16_t) 0;
	// dbg_rows |= (USB->ISTR & USB_ISTR_CTR)  ? (*k.hand)[2] : (uint16_t) 0;
	// dbg_rows |= (USB->ISTR & USB_ISTR_SUSP) ? (*k.hand)[3] : (uint16_t) 0;
	// dbg_rows |= (USB->ISTR & USB_ISTR_WKUP) ? (*k.hand)[4] : (uint16_t) 0;
	// dbg_rows |= (USB->ISTR & USB_ISTR_ERR)  ? (*k.hand)[5] : (uint16_t) 0;
	// LL_GPIO_SetOutputPin(GPIOB, dbg_rows);

	if (USB->ISTR & USB_ISTR_SUSP) {
		SuspendCallback();
	}
	if (USB->ISTR & USB_ISTR_WKUP) {
		ResumeCallback();
	}
	// LL_GPIO_ResetOutputPin(GPIOB, k_all_rows(&k));
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
	    LL_EXTI_LINE_0 | LL_EXTI_LINE_1 | LL_EXTI_LINE_2 | LL_EXTI_LINE_3 |
	    LL_EXTI_LINE_4 | LL_EXTI_LINE_5 | LL_EXTI_LINE_6 | LL_EXTI_LINE_7;
	EXTI_InitStruct.LineCommand = ENABLE;
	EXTI_InitStruct.Mode        = LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger     = LL_EXTI_TRIGGER_RISING;
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
	    LL_EXTI_LINE_0 | LL_EXTI_LINE_1 | LL_EXTI_LINE_2 | LL_EXTI_LINE_3 |
	    LL_EXTI_LINE_4 | LL_EXTI_LINE_5 | LL_EXTI_LINE_6 | LL_EXTI_LINE_7;
	EXTI_InitStruct.LineCommand = DISABLE;
	EXTI_InitStruct.Mode        = LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger     = LL_EXTI_TRIGGER_NONE;
	LL_EXTI_Init(&EXTI_InitStruct);
}

static void UART_AS_INT() {
	LL_EXTI_InitTypeDef EXTI_InitStruct = {0};

	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE9);

	EXTI_InitStruct.Line_0_31   = LL_EXTI_LINE_9;
	EXTI_InitStruct.LineCommand = ENABLE;
	EXTI_InitStruct.Mode        = LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger     = LL_EXTI_TRIGGER_RISING;
	LL_EXTI_Init(&EXTI_InitStruct);

	NVIC_SetPriority(EXTI4_15_IRQn, 0);
	NVIC_EnableIRQ(EXTI4_15_IRQn);
}

static void UART_AS_SINGLE_WIRE() {
	LL_EXTI_InitTypeDef EXTI_InitStruct = {0};

	EXTI_InitStruct.Line_0_31   = LL_EXTI_LINE_9;
	EXTI_InitStruct.LineCommand = DISABLE;
	EXTI_InitStruct.Mode        = LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger     = LL_EXTI_TRIGGER_NONE;
	LL_EXTI_Init(&EXTI_InitStruct);
}

static void UART_CLEAR_IT() {
	__HAL_UART_FLUSH_DRREGISTER(&huart1);
	__HAL_UART_CLEAR_FLAG(&huart1, UART_CLEAR_TCF | UART_CLEAR_IDLEF |
	                                   UART_CLEAR_OREF | UART_CLEAR_NEF |
	                                   UART_CLEAR_PEF | UART_CLEAR_FEF);
}

static void UART_AS_TX() {
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_NO);
	LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_9, LL_GPIO_OUTPUT_PUSHPULL);
	HAL_HalfDuplex_EnableTransmitter(&huart1);
	UART_CLEAR_IT();
}

static uint32_t wait_stable_idle(uint32_t t) {
	UNUSED(t);
	// TODO
	// don't loop forever, in case something pulls down the line
	//
	for (int i = 0, n = 0; n < t; n++) {
		if (LL_GPIO_ReadInputPort(GPIOA) & LL_GPIO_PIN_9) {
			if (++i > 8) {
				return 1;
			};
		} else {
			i = 0;
		}
	}

	return 0;
}

static void UART_AS_RX() {
	// clear the receive buffer to not receive garbage or carry old data arounf
	w_clear(&k.MsgBuf);

	LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_9, LL_GPIO_OUTPUT_OPENDRAIN);
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_UP);

	// Make SURE we read a stable high input, or UART will falsly detect a
	// start bit.
	//
	// One transfer takes about 150uS.
	//
	if (!wait_stable_idle(256)) {
		return;
	}
	HAL_HalfDuplex_EnableReceiver(&huart1);
	UART_CLEAR_IT();
}

static void Report_UART() {
	LL_TIM_DisableCounter(TIM15);
	TIM15->CNT = 0;
	LL_TIM_EnableCounter(TIM15);
	timeout_ctr = TIMEOUT_HAVE_UART;

	GPIO_AS_INPUT();

	if (get_rows(k.MsgBuf.rows) == 0) {
		// no need to send the state if no button is pressed
		return;
	}
	w_crc_fill(&k.MsgBuf);

	if (complete) {
		complete = 0;
		// also wait here for stable line, so we don't send before receiver is
		// ready
		if (!wait_stable_idle(256)) {
			return;
		}
		UART_AS_SINGLE_WIRE();
		UART_AS_TX();
		HAL_UART_Transmit_DMA(&huart1, k.MsgBuf.bytes, sizeof(k.MsgBuf.bytes));
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	complete = 1;
	UART_AS_INT();
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_NO);
	w_clear(&k.MsgBuf);
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_DOWN);
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
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_9) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_9);
		switch (timeout_ctr) {
		case TIMEOUT_HAVE_USB:
			break;
		case TIMEOUT_HAVE_UART:
			Report_UART();
			break;
		case TIMEOUT_SUSPEND:
			Remote_Wake();
			break;
		}
		return;
	}
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
