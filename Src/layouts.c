#include <string.h>

#include "app.h"
#include "bits.h"
#include "flash.h"
#include "layouts.h"
#include "main.h"

// clang-format off
const Layout_t LAYOUT_SELECT = {
    /* Left hand */
    { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
    { 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u},
    { 9u,10u,11u,12u,13u,14u,15u,16u},
    {17u,18u,19u,20u,21u,22u,23u,24u},
    {25u,26u,27u,28u,29u,30u,31u,32u},
    {33u,34u,35u,36u,37u,38u,39u,40u},
    /* Right hand */                
    { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
    { 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u},
    { 9u,10u,11u,12u,13u,14u,15u,16u},
    {17u,18u,19u,20u,21u,22u,23u,24u},
    {25u,26u,27u,28u,29u,30u,31u,32u},
    {33u,34u,35u,36u,37u,38u,39u,40u},
};

const Layout_t LAYOUT_DEFAULT = {
    /* Left hand */
    { UKEY_ESCAPE , UKEY_F01   , UKEY_F02          , UKEY_F03              , UKEY_F04       , UKEY_B           , UKEY_F05       , UKEY_F06      },
    { 0u          , 0u         , UKEY_GRAVE        , UKEY_1                , UKEY_2         , UKEY_3           , UKEY_4         , UKEY_5        },
    { 0u          , 0u         , UKEY_TAB          , UKEY_Q                , UKEY_W         , UKEY_E           , UKEY_R         , UKEY_T        },
    { 0u          , 0u         , UKEY_INSERT       , UKEY_A                , UKEY_S         , UKEY_D           , UKEY_F         , UKEY_G        },
    { 0u          , 0u         , UKEY_LEFT_SHIFT   , UKEY_BACKSLASH_NON_US , UKEY_Z         , UKEY_X           , UKEY_C         , UKEY_V        },
    { 0u          , 0u         , UKEY_LEFT_CONTROL , UKEY_PRINT            , UKEY_LEFT_GUI  , UKEY_LEFT_ALT    , UKEY_SPACE     , UKEY_ENTER    },
    /* Right hand */
    { UKEY_F07    , UKEY_F08   , UKEY_B            , UKEY_F09              , UKEY_F10       , UKEY_F11         , UKEY_F12       , UKEY_PAD_NUM  },
    { UKEY_6      , UKEY_7     , UKEY_8            , UKEY_9                , UKEY_0         , UKEY_MINUS       , UKEY_EQUAL     , UKEY_HOME     },
    { UKEY_Y      , UKEY_U     , UKEY_I            , UKEY_O                , UKEY_P         , UKEY_LBRACKET    , UKEY_RBRACKET  , UKEY_END      },
    { UKEY_H      , UKEY_J     , UKEY_K            , UKEY_L                , UKEY_SEMICOLON , UKEY_APOSTROPHE  , UKEY_BACKSLASH , UKEY_PAGEUP   },
    { UKEY_N      , UKEY_M     , UKEY_COMMA        , UKEY_PERIOD           , UKEY_SLASH     , UKEY_RIGHT_SHIFT , UKEY_UP        , UKEY_PAGEDOWN },
    { UKEY_ENTER  , UKEY_SPACE , UKEY_DELETE       , UKEY_DELETEFORWARD    , UKEY_RIGHT_ALT , UKEY_LEFT        , UKEY_DOWN      , UKEY_RIGHT    },
};

const Layout_t LAYOUT_DEFAULT_NUM = {
    /* Left hand */
    { UKEY_ESCAPE  , UKEY_F01     , UKEY_F02           , UKEY_F03              , UKEY_F04       , UKEY_B           , UKEY_F05       , UKEY_F06      },
    { 0u           , 0u           , UKEY_GRAVE         , UKEY_1                , UKEY_2         , UKEY_3           , UKEY_4         , UKEY_5        },
    { 0u           , 0u           , UKEY_TAB           , UKEY_Q                , UKEY_W         , UKEY_E           , UKEY_R         , UKEY_T        },
    { 0u           , 0u           , UKEY_INSERT        , UKEY_A                , UKEY_S         , UKEY_D           , UKEY_F         , UKEY_G        },
    { 0u           , 0u           , UKEY_LEFT_SHIFT    , UKEY_BACKSLASH_NON_US , UKEY_Z         , UKEY_X           , UKEY_C         , UKEY_V        },
    { 0u           , 0u           , UKEY_LEFT_CONTROL  , UKEY_PRINT            , UKEY_LEFT_GUI  , UKEY_LEFT_ALT    , UKEY_SPACE     , UKEY_ENTER    },
    /* Right hand */
    { UKEY_F07     , UKEY_F08     , UKEY_DELETE        , UKEY_F09              , UKEY_F10       , UKEY_F11         , UKEY_F12       , UKEY_PAD_NUM  },
    { UKEY_PAD_NUM , UKEY_PAD_DIV , UKEY_PAD_MUL       , UKEY_PAD_SUB          , UKEY_0         , UKEY_MINUS       , UKEY_EQUAL     , UKEY_HOME     },
    { UKEY_PAD_7   , UKEY_PAD_8   , UKEY_PAD_9         , UKEY_PAD_ADD          , UKEY_P         , UKEY_LBRACKET    , UKEY_RBRACKET  , UKEY_END      },
    { UKEY_PAD_4   , UKEY_PAD_5   , UKEY_PAD_6         , UKEY_PAD_ADD          , UKEY_SEMICOLON , UKEY_APOSTROPHE  , UKEY_BACKSLASH , UKEY_PAGEUP   },
    { UKEY_PAD_1   , UKEY_PAD_2   , UKEY_PAD_3         , UKEY_PAD_ENTER        , UKEY_SLASH     , UKEY_RIGHT_SHIFT , UKEY_UP        , UKEY_PAGEDOWN },
    { UKEY_ENTER   , UKEY_PAD_0   , UKEY_PAD_SEPARATOR , UKEY_PERIOD           , UKEY_RIGHT_ALT , UKEY_LEFT        , UKEY_DOWN      , UKEY_RIGHT    },
};

const uint16_t ALL_ROWS[2*ROWS] = {
    GPIO_LEFT_0_Pin,
    GPIO_LEFT_1_Pin,
    GPIO_LEFT_2_Pin,
    GPIO_LEFT_3_Pin,
    GPIO_LEFT_4_Pin,
    GPIO_LEFT_5_Pin,
    GPIO_RIGHT_0_Pin,
    GPIO_RIGHT_1_Pin,
    GPIO_RIGHT_2_Pin,
    GPIO_RIGHT_3_Pin,
    GPIO_RIGHT_4_Pin,
    GPIO_RIGHT_5_Pin,
};

const uint16_t lyt_all_rows =
GPIO_LEFT_0_Pin | GPIO_LEFT_1_Pin | GPIO_LEFT_2_Pin |
GPIO_LEFT_3_Pin | GPIO_LEFT_4_Pin | GPIO_LEFT_5_Pin |
GPIO_RIGHT_0_Pin | GPIO_RIGHT_1_Pin | GPIO_RIGHT_2_Pin |
GPIO_RIGHT_3_Pin | GPIO_RIGHT_4_Pin | GPIO_RIGHT_5_Pin
;

// clang-format on

const Layout_t *Layouts[] = {
    (&LAYOUT_SELECT),
    (&LAYOUT_DEFAULT),
};

#define N_Layouts (sizeof(Layouts) / sizeof(Layouts[0]))

const Layout_t *Layouts_num[] = {
    (&LAYOUT_SELECT),
    (&LAYOUT_DEFAULT_NUM),
};

const Layout_t *lyt_get_layout(uint8_t layout_idx) {
	return Layouts[layout_idx];
};
const Layout_t *lyt_get_layout_num(uint8_t layout_idx) {
	return Layouts_num[layout_idx];
};

static uint8_t lyt_modmask(const uint8_t k) {
	switch (k) {
	case UKEY_LEFT_SHIFT:
		return UKEY_BIT_LEFT_SHIFT;
	case UKEY_LEFT_CONTROL:
		return UKEY_BIT_LEFT_CONTROL;
	case UKEY_LEFT_GUI:
		return UKEY_BIT_LEFT_GUI;
	case UKEY_LEFT_ALT:
		return UKEY_BIT_LEFT_ALT;
	case UKEY_RIGHT_SHIFT:
		return UKEY_BIT_RIGHT_SHIFT;
	case UKEY_RIGHT_CONTROL:
		return UKEY_BIT_RIGHT_CONTROL;
	case UKEY_RIGHT_GUI:
		return UKEY_BIT_RIGHT_GUI;
	case UKEY_RIGHT_ALT:
		return UKEY_BIT_RIGHT_ALT;
	default:
		return 0u;
	}
}

static uint8_t lyt_is_modkey(const uint8_t k) {
	return k >= UKEY_LEFT_SHIFT && k <= UKEY_RIGHT_GUI;
}

uint8_t lyt_report_boot(const Layout_t *layout,
                        uint8_t         report[8],
                        const uint8_t   rows[static ROWS]) {
	uint8_t  n      = 0u;
	uint8_t *moddst = NULL;
	uint8_t *dst    = NULL;

	if (report != NULL) {
		moddst = &report[0u];
		dst    = &report[2u];
	}

	for (uint8_t r = 0u; r < ROWS; r++) {
		if (rows[r] == 0u) {
			continue;
		}

		for (uint8_t c = 0u; c < COLS; c++) {
			uint8_t key;

			if (!(rows[r] & (1u << c))) {
				continue;
			}

			key = (*layout)[r][c];
			if (!key) {
				continue;
			}

			if (lyt_is_modkey(key)) {
				if (moddst != NULL) {
					*moddst |= lyt_modmask(key);
				}
				continue;
			}

			if (n == 6u) {
				if (dst != NULL) {
					memset(dst, UKEY_ERROR_ROLL_OVER, 6ul);
				}
				return 6u;
			}

			if (dst != NULL) {
				dst[n] = key;
			}

			n++;
		}
	}

	return n;
}

uint8_t lyt_report_bits(const Layout_t *layout,
                        Bits_t          report,
                        const uint8_t   rows[static ROWS]) {
	uint8_t n = 0u;

	for (uint8_t r = 0u; r < ROWS; r++) {
		if (rows[r] == 0u) {
			continue;
		}

		for (uint8_t c = 0u; c < COLS; c++) {
			uint8_t key;

			if (!(rows[r] & (1u << c))) {
				continue;
			}

			key = (*layout)[r][c];
			if (!key) {
				continue;
			}

			if (report != NULL) {
				bits_set(report, key);
			}
			n++;
		}
	}

	return n;
}

static uint8_t lyt_count_pressed(const Layout_t *layout,
                                 const uint8_t   rows[static ROWS]) {
	return lyt_report_bits(layout, NULL, rows);
}

uint8_t lyt_select_layout() {
	uint8_t rows[ROWS];
	uint8_t report[8];
	uint8_t layout_idx = 0u;

	// check if we have a layout saved to flash
	{
		uint16_t saved = flash_layout_load();
		if (saved != 0xFFFFU) {
			layout_idx = saved & 0xFFU;
			if (layout_idx > 0u && layout_idx < N_Layouts) {
				return layout_idx;
			}
		}
	}

	while (1u) {
		HAL_Delay(1u);
		get_rows(rows);

		// wait until there is only a single button pressed
		if (lyt_report_boot(Layouts[0u], report, rows) != 1u) {
			continue;
		}

		// get the selected layout
		layout_idx = report[2u];

		// check if there is a valid layout for the pressed button
		if (layout_idx > 0u && layout_idx < N_Layouts) {
			break;
		}
	}

	// save the config
	flash_layout_append((uint16_t) layout_idx);

	// wait until no button is pressed, otherwise we start typing right away
	do {
		HAL_Delay(1u);
		get_rows(rows);
	} while (lyt_count_pressed(lyt_get_layout(layout_idx), rows) > 0u);

	return layout_idx;
}

static uint8_t get_row(const uint16_t r) {
	uint8_t ret;

	// wait until evenything is drained
	while (LL_GPIO_ReadInputPort(GPIOA) & 0xFFU) {}

	// select the row to scan
	LL_GPIO_SetOutputPin(GPIOB, r);

	// read out all the pressed buttons in the selected row
	ret = (uint8_t)(LL_GPIO_ReadInputPort(GPIOA) & 0xFFU);

	// deselct the row
	LL_GPIO_ResetOutputPin(GPIOB, r);

	return ret;
}

uint32_t get_rows(uint8_t dst[static ROWS]) {
	const uint16_t lyt_all_rows_left = GPIO_LEFT_0_Pin | GPIO_LEFT_1_Pin |
	                                   GPIO_LEFT_2_Pin | GPIO_LEFT_3_Pin |
	                                   GPIO_LEFT_4_Pin | GPIO_LEFT_5_Pin;
	const uint16_t lyt_all_rows_right = GPIO_RIGHT_0_Pin | GPIO_RIGHT_1_Pin |
	                                    GPIO_RIGHT_2_Pin | GPIO_RIGHT_3_Pin |
	                                    GPIO_RIGHT_4_Pin | GPIO_RIGHT_5_Pin;

	// If a button press is observed on one half, unconditionally zero the
	// other half.  €ach half takes about 10us to scan, so this approach saves
	// around 20us scanning time if no buttons are pressed and about 10 if
	// there are buttons pressed over just scanning everything.

	// Test left hand
	LL_GPIO_SetOutputPin(GPIOB, lyt_all_rows_left);
	if ((LL_GPIO_ReadInputPort(GPIOA) & 0xFFU)) {
		LL_GPIO_ResetOutputPin(GPIOB, lyt_all_rows);
		for (uint32_t r = 0u; r < ROWS / 2u; r++) {
			dst[r] = get_row(ALL_ROWS[r]);
		}
		// this is the left board, zero the other half
		memset(&dst[ROWS / 2u], 0, ROWS / 2u);
		return 1u;
	}

	// Test right hand
	LL_GPIO_SetOutputPin(GPIOB, lyt_all_rows_right);
	if ((LL_GPIO_ReadInputPort(GPIOA) & 0xFFU)) {
		LL_GPIO_ResetOutputPin(GPIOB, lyt_all_rows);
		for (uint32_t r = ROWS / 2u; r < ROWS; r++) {
			dst[r] = get_row(ALL_ROWS[r]);
		}
		// this is the right board, zero the other half
		memset(dst, 0, ROWS / 2u);
		return 1u;
	}

	LL_GPIO_ResetOutputPin(GPIOB, lyt_all_rows);
	memset(dst, 0, ROWS);
	return 0u;
}
