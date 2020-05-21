#include <string.h>

#include "app.h"
#include "bits.h"
#include "flash.h"
#include "layouts.h"
#include "main.h"

// clang-format off
const Layout_t LAYOUT_SELECT = {
    /* Left hand */
    { 0, 0, 0, 0, 0, 0, 0, 0},
    { 1, 2, 3, 4, 5, 6, 7, 8},
    { 9,10,11,12,13,14,15,16},
    {17,18,19,20,21,22,23,24},
    {25,26,27,28,29,30,31,32},
    {33,34,35,36,37,38,39,40},
    /* Right hand */
    { 0, 0, 0, 0, 0, 0, 0, 0},
    { 1, 2, 3, 4, 5, 6, 7, 8},
    { 9,10,11,12,13,14,15,16},
    {17,18,19,20,21,22,23,24},
    {25,26,27,28,29,30,31,32},
    {33,34,35,36,37,38,39,40},
};

const Layout_t LAYOUT_DEFAULT = {
    /* Left hand */
    { UKEY_ESCAPE , UKEY_F01   , UKEY_F02          , UKEY_F03              , UKEY_F04       , UKEY_B           , UKEY_F05       , UKEY_F06      },
    { 0           , 0          , UKEY_GRAVE        , UKEY_1                , UKEY_2         , UKEY_3           , UKEY_4         , UKEY_5        },
    { 0           , 0          , UKEY_TAB          , UKEY_Q                , UKEY_W         , UKEY_E           , UKEY_R         , UKEY_T        },
    { 0           , 0          , UKEY_INSERT       , UKEY_A                , UKEY_S         , UKEY_D           , UKEY_F         , UKEY_G        },
    { 0           , 0          , UKEY_LEFT_SHIFT   , UKEY_BACKSLASH_NON_US , UKEY_Z         , UKEY_X           , UKEY_C         , UKEY_V        },
    { 0           , 0          , UKEY_LEFT_CONTROL , UKEY_RIGHT_ALT        , UKEY_LEFT_GUI  , UKEY_LEFT_ALT    , UKEY_SPACE     , UKEY_ENTER    },
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
    { 0            , 0            , UKEY_GRAVE         , UKEY_1                , UKEY_2         , UKEY_3           , UKEY_4         , UKEY_5        },
    { 0            , 0            , UKEY_TAB           , UKEY_Q                , UKEY_W         , UKEY_E           , UKEY_R         , UKEY_T        },
    { 0            , 0            , UKEY_INSERT        , UKEY_A                , UKEY_S         , UKEY_D           , UKEY_F         , UKEY_G        },
    { 0            , 0            , UKEY_LEFT_SHIFT    , UKEY_BACKSLASH_NON_US , UKEY_Z         , UKEY_X           , UKEY_C         , UKEY_V        },
    { 0            , 0            , UKEY_LEFT_CONTROL  , UKEY_RIGHT_ALT        , UKEY_LEFT_GUI  , UKEY_LEFT_ALT    , UKEY_SPACE     , UKEY_ENTER    },
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
		return 0;
	}
}

static uint8_t lyt_is_modkey(const uint8_t k) {
	return k >= UKEY_LEFT_SHIFT && k <= UKEY_RIGHT_GUI;
}

uint8_t lyt_report_boot(const Layout_t *layout,
                        uint8_t         report[8],
                        const uint8_t   rows[ROWS]) {
	uint8_t  n      = 0;
	uint8_t *moddst = NULL;
	uint8_t *dst    = NULL;

	if (report != NULL) {
		moddst = &report[0];
		dst    = &report[2];
	}

	for (uint8_t r = 0; r < ROWS; r++) {
		if (rows[r] == 0) {
			continue;
		}

		for (uint8_t c = 0; c < COLS; c++) {
			uint8_t key;

			if (!(rows[r] & (1 << c))) {
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

			if (n == 6) {
				if (dst != NULL) {
					memset(dst, UKEY_ERROR_ROLL_OVER, 6);
				}
				return 6;
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
                        const uint8_t   rows[ROWS]) {
	uint8_t n = 0;

	for (uint8_t r = 0; r < ROWS; r++) {
		if (rows[r] == 0) {
			continue;
		}

		for (uint8_t c = 0; c < COLS; c++) {
			uint8_t key;

			if (!(rows[r] & (1 << c))) {
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
                                 const uint8_t   rows[ROWS]) {
	return lyt_report_bits(layout, NULL, rows);
}

uint8_t lyt_select_layout() {
	uint8_t rows[ROWS];
	uint8_t report[8];
	uint8_t layout_idx = 0;

	// check if we have a layout saved to flash
	{
		uint16_t saved = flash_layout_load();
		if (saved != 0xFFFFU) {
			layout_idx = (saved >> 0) & 0xFF;
			if (layout_idx > 0 && layout_idx < N_Layouts) {
				return layout_idx;
			}
		}
	}

	while (1) {
		HAL_Delay(1);
		get_rows(rows);

		// wait until there is only a single button pressed
		if (lyt_report_boot(Layouts[0], report, rows) != 1) {
			continue;
		}

		// get the selected layout
		layout_idx = report[2];

		// check if there is a valid layout for the pressed button
		if (layout_idx > 0 && layout_idx < N_Layouts) {
			break;
		}
	}

	// save the config
	flash_layout_append((uint16_t) layout_idx);

	// wait until no button is pressed, otherwise we start typing right away
	do {
		HAL_Delay(1);
		get_rows(rows);
	} while (lyt_count_pressed(lyt_get_layout(layout_idx), rows) > 0);

	return layout_idx;
}

static uint8_t get_row(const uint16_t r) {
	uint8_t ret;

	// wait until evenything is drained
	while (LL_GPIO_ReadInputPort(GPIOA) & 0xFF) {}

	// select the row to scan
	LL_GPIO_SetOutputPin(GPIOB, r);

	// read out all the pressed buttons in the selected row
	ret = (uint8_t)(LL_GPIO_ReadInputPort(GPIOA) & 0xFF);

	// deselct the row
	LL_GPIO_ResetOutputPin(GPIOB, r);

	return ret;
}

void get_rows(uint8_t dst[ROWS]) {
	int r;
	for (r = 0; r < ROWS; r++) {
		dst[r] = get_row(ALL_ROWS[r]);
	}
}
