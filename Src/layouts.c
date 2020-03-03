#include "layouts.h"
#include "main.h"

Layout_t LAYOUT_SELECT = {
    { 0, 0, 0, 0, 0, 0, 0, 0},
    { 1, 2, 3, 4, 5, 6, 7, 8},
    { 9,10,11,12,13,14,15,16},
    {17,18,19,20,21,22,23,24},
    {25,26,27,28,29,30,31,32},
    {33,34,35,36,37,38,39,40},
};

Layout_t LAYOUT_LEFT = {
     { UKEY_ESCAPE , UKEY_F01 , UKEY_F02        , UKEY_F03              , UKEY_F04      , UKEY_B        , UKEY_F05   , UKEY_F06   },
     { 0           , 0        , UKEY_GRAVE      , UKEY_1                , UKEY_2        , UKEY_3        , UKEY_4     , UKEY_5     },
     { 0           , 0        , UKEY_TAB        , UKEY_Q                , UKEY_W        , UKEY_E        , UKEY_R     , UKEY_T     },
     { 0           , 0        , 0               , UKEY_A                , UKEY_S        , UKEY_D        , UKEY_F     , UKEY_G     },
     { 0           , 0        , UKEY_LEFT_SHIFT , UKEY_BACKSLASH_NON_US , UKEY_Z        , UKEY_X        , UKEY_C     , UKEY_V     },
     { 0           , 0        , 0               , UKEY_LEFT_CONTROL     , UKEY_LEFT_GUI , UKEY_LEFT_ALT , UKEY_ENTER , UKEY_SPACE },
};

Layout_t LAYOUT_RIGHT = {
    { UKEY_F07   , UKEY_F08   , UKEY_B      , UKEY_F09           , UKEY_F10       , UKEY_F11         , UKEY_F12       , 0           },
    { UKEY_6     , UKEY_7     , UKEY_8      , UKEY_9             , UKEY_0         , UKEY_MINUS       , UKEY_EQUAL     , UKEY_DELETE },
    { UKEY_Y     , UKEY_U     , UKEY_I      , UKEY_O             , UKEY_P         , UKEY_LBRACKET    , UKEY_RBRACKET  , 0           },
    { UKEY_H     , UKEY_J     , UKEY_K      , UKEY_L             , UKEY_SEMICOLON , UKEY_APOSTROPHE  , UKEY_BACKSLASH , UKEY_ENTER  },
    { UKEY_N     , UKEY_M     , UKEY_COMMA  , UKEY_PERIOD        , UKEY_SLASH     , UKEY_RIGHT_SHIFT , UKEY_UP        , 0           },
    { UKEY_SPACE , UKEY_ENTER , UKEY_DELETE , UKEY_DELETEFORWARD , UKEY_RIGHT_ALT , UKEY_LEFT        , UKEY_DOWN      , UKEY_RIGHT  },
};

Layout_t LAYOUT_LEFT_ADNW = {
    { UKEY_ESCAPE , UKEY_F01          , UKEY_F02              , UKEY_F03      , UKEY_F04   , 0             , UKEY_F05 , UKEY_F06   },
    { 0           , 0                 , UKEY_GRAVE            , UKEY_1        , UKEY_2     , UKEY_3        , UKEY_4   , UKEY_5     },
    { 0           , 0                 , UKEY_TAB              , UKEY_Q        , UKEY_W     , UKEY_E        , UKEY_R   , UKEY_T     },
    { 0           , 0                 , 0                     , UKEY_A        , UKEY_S     , UKEY_D        , UKEY_F   , UKEY_G     },
    { 0           , UKEY_LEFT_SHIFT   , UKEY_BACKSLASH_NON_US , UKEY_Z        , UKEY_X     , UKEY_C        , UKEY_V   , UKEY_B     },
    { 0           , UKEY_LEFT_CONTROL , UKEY_LEFT_GUI         , UKEY_LEFT_ALT , UKEY_ENTER , UKEY_LEFT_GUI , UKEY_TAB , UKEY_SPACE },
};

Layout_t *Layouts[] = {
    (&LAYOUT_SELECT),
    (&LAYOUT_LEFT),
    (&LAYOUT_RIGHT),
    (&LAYOUT_LEFT_ADNW),
};

#define N_Layouts (sizeof(Layouts) / sizeof(Layouts[0]))

const uint16_t ROWS_HAND[2][ROWS] = {
{
    GPIO_PIN_10,
    GPIO_PIN_11,
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14,
    GPIO_PIN_15,
},
{
    GPIO_PIN_9,
    GPIO_PIN_8,
    GPIO_PIN_7,
    GPIO_PIN_6,
    GPIO_PIN_5,
    GPIO_PIN_4,
}
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
    return lyt_modmask(k) > 0;
}

uint8_t lyt_get_pressed(Layout_t *layout, uint8_t dst[NSWITCH], uint8_t *moddst, const uint8_t rows[ROWS]) {
    uint8_t n = 0;

    for (uint8_t r = 0; r < ROWS; r++) {
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

            if (dst != NULL) {
                dst[n] = key;
            }

            n++;
        }
    }

    return n;
}

static uint8_t lyt_count_pressed(Layout_t *layout, const uint8_t rows[ROWS]) {
    return lyt_get_pressed(layout, NULL, NULL, rows);
}

void lyt_select_layout(Layout_t **lyt, const uint16_t (**hand)[ROWS]) {
    uint8_t rows[ROWS];
    uint8_t buttons[NSWITCH];
    uint8_t hand_idx = 1;

    while (1) {
        uint8_t layout_idx;

        hand_idx = !hand_idx;
        *hand = &ROWS_HAND[hand_idx];

        HAL_Delay(1);
        get_rows(rows, **hand);

        // wait until there is only a single button pressed
        if (lyt_get_pressed(Layouts[0], buttons, NULL, rows) != 1) {
            continue;
        }

        // get the selected layout
        layout_idx = buttons[0];

        // check if there is a valid layout for the pressed button
        if (layout_idx < 1 || layout_idx >= N_Layouts) {
            continue;
        }

        // wait until no button is pressed, otherwise we start typing right away
        do {
            HAL_Delay(1);
            get_rows(rows, **hand);
        } while (lyt_count_pressed(Layouts[0], rows) > 0);

        *lyt = Layouts[layout_idx];
        return;
    }
}

static uint8_t get_row(const uint16_t r) {
    uint8_t ret;

    // select the row to scan
    HAL_GPIO_WritePin(GPIOB, r, GPIO_PIN_SET);

    // add some delay to not miss any input
    for (int i = 0; i < 8; i++) {
        __NOP();
    }

    // read out all the pressed buttons in the selected row
    ret = (uint8_t) (GPIOA->IDR & 0xFF);

    // deselct the row
    HAL_GPIO_WritePin(GPIOB, r, GPIO_PIN_RESET);
    return ret;
}


void get_rows(uint8_t dst[ROWS], const uint16_t hand[ROWS]) {
    dst[0] = get_row(hand[0]);
    dst[1] = get_row(hand[1]);
    dst[2] = get_row(hand[2]);
    dst[3] = get_row(hand[3]);
    dst[4] = get_row(hand[4]);
    dst[5] = get_row(hand[5]);
}
