#include "layouts.h"

uint8_t LAYOUT_LEFT[ROWS][COLS] = {
	   { UKEY_ESCAPE, UKEY_F01         , UKEY_F02             , UKEY_F03     , UKEY_F04  , 0     , UKEY_F05, UKEY_F06  },
	   { 0          , 0                , UKEY_GRAVE           , UKEY_1       , UKEY_2    , UKEY_3, UKEY_4  , UKEY_5    },
	   { 0          , 0                , UKEY_TAB             , UKEY_Q       , UKEY_W    , UKEY_E, UKEY_R  , UKEY_T    },
	   { 0          , 0                , 0                    , UKEY_A       , UKEY_S    , UKEY_D, UKEY_F  , UKEY_G    },
	   { 0          , UKEY_LEFT_SHIFT  , UKEY_BACKSLASH_NON_US, UKEY_Z       , UKEY_X    , UKEY_C, UKEY_V  , UKEY_B    },
	   { 0          , UKEY_LEFT_CONTROL, UKEY_LEFT_GUI        , UKEY_LEFT_ALT, UKEY_ENTER, UKEY_LEFT_GUI, UKEY_TAB  , UKEY_SPACE},
};

uint8_t LAYOUT_RIGHT[ROWS][COLS] = {
	 { UKEY_F07   , UKEY_F08    , 0                  , UKEY_F09    , UKEY_F10       , UKEY_F11         , UKEY_F12      , 0    },
	 { UKEY_6     , UKEY_7      , UKEY_8             , UKEY_9      , UKEY_0         , UKEY_MINUS       , UKEY_EQUAL    , UKEY_DELETE },
	 { UKEY_Y     , UKEY_U      , UKEY_I             , UKEY_O      , UKEY_P         , UKEY_LBRACKET    , UKEY_RBRACKET , UKEY_ENTER     },
	 { UKEY_H     , UKEY_J      , UKEY_K             , UKEY_L      , UKEY_SEMICOLON , UKEY_APOSTROPHE  , UKEY_BACKSLASH     , UKEY_ENTER     },
	 { UKEY_N     , UKEY_M      , UKEY_COMMA         , UKEY_PERIOD , UKEY_SLASH     , UKEY_RIGHT_SHIFT , UKEY_UP       , 0              },
	 { UKEY_SPACE , UKEY_DELETE , UKEY_DELETEFORWARD , UKEY_ENTER  , UKEY_RIGHT_ALT , UKEY_LEFT        , UKEY_DOWN     , UKEY_RIGHT     },
};

#ifdef RIGHT_HAND
#define LAYOUT LAYOUT_RIGHT
#else
#define LAYOUT LAYOUT_LEFT
#endif

uint8_t modmask(const uint8_t k) {
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

uint8_t is_modkey(const uint8_t k) {
    return modmask(k) > 0;
}

uint8_t get_pressed(uint8_t dst[NSWITCH], uint8_t *moddst, const uint8_t rows[ROWS]) {
    uint8_t n = 0;
    for (uint8_t r = 0; r < ROWS; r++) {
        for (uint8_t c = 0; c < COLS; c++) {
            if (rows[r] & (1 << c)) {
                uint8_t key = LAYOUT[r][c];
                if (!key) {
                    continue;
                }
                if (is_modkey(key)) {
                    *moddst |= modmask(key);
                } else {
                    dst[n] = key;
                    n++;
                }
            }
        }
    }

    return n;
}
