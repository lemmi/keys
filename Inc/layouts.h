#ifndef LAYOUTS_H
#define LAYOUTS_H

#include <stdint.h>

#define ROWS 6
#define COLS 8
#define NSWITCH (ROWS * COLS)


// https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf

#define UKEY_ERROR_ROLL_OVER 1

#define UKEY_A 4
#define UKEY_B 5
#define UKEY_C 6
#define UKEY_D 7
#define UKEY_E 8
#define UKEY_F 9
#define UKEY_G 10
#define UKEY_H 11
#define UKEY_I 12
#define UKEY_J 13
#define UKEY_K 14
#define UKEY_L 15
#define UKEY_M 16
#define UKEY_N 17
#define UKEY_O 18
#define UKEY_P 19
#define UKEY_Q 20
#define UKEY_R 21
#define UKEY_S 22
#define UKEY_T 23
#define UKEY_U 24
#define UKEY_V 25
#define UKEY_W 26
#define UKEY_X 27
#define UKEY_Y 28
#define UKEY_Z 29

#define UKEY_1 30
#define UKEY_2 31
#define UKEY_3 32
#define UKEY_4 33
#define UKEY_5 34
#define UKEY_6 35
#define UKEY_7 36
#define UKEY_8 37
#define UKEY_9 38
#define UKEY_0 39

#define UKEY_ENTER 40
#define UKEY_ESCAPE 41
#define UKEY_DELETE 42
#define UKEY_TAB 43
#define UKEY_SPACE 44
#define UKEY_MINUS 45
#define UKEY_EQUAL 46
#define UKEY_LBRACKET 47
#define UKEY_RBRACKET 48
#define UKEY_BACKSLASH 49
#define UKEY_HASH 50
#define UKEY_SEMICOLON 51
#define UKEY_APOSTROPHE 52
#define UKEY_GRAVE 53
#define UKEY_COMMA 54
#define UKEY_PERIOD 55
#define UKEY_SLASH 56
#define UKEY_CAPS 57

#define UKEY_F01 58
#define UKEY_F02 59
#define UKEY_F03 60
#define UKEY_F04 61
#define UKEY_F05 62
#define UKEY_F06 63
#define UKEY_F07 64
#define UKEY_F08 65
#define UKEY_F09 66
#define UKEY_F10 67
#define UKEY_F11 68
#define UKEY_F12 69

#define UKEY_PRINT 70
#define UKEY_SCROLL 71
#define UKEY_PAUSE 72

#define UKEY_INSERT 73
#define UKEY_HOME 74
#define UKEY_PAGEUP 75
#define UKEY_DELETEFORWARD 76
#define UKEY_END 77
#define UKEY_PAGEDOWN 78

#define UKEY_RIGHT 79
#define UKEY_LEFT 80
#define UKEY_DOWN 81
#define UKEY_UP 82

#define UKEY_PAD_NUM 83
#define UKEY_PAD_DIV 84
#define UKEY_PAD_MUL 85
#define UKEY_PAD_SUB 86
#define UKEY_PAD_ADD 87
#define UKEY_PAD_ENTER 88

#define UKEY_PAD_1 89
#define UKEY_PAD_2 90
#define UKEY_PAD_3 91
#define UKEY_PAD_4 92
#define UKEY_PAD_5 93
#define UKEY_PAD_6 94
#define UKEY_PAD_7 95
#define UKEY_PAD_8 96
#define UKEY_PAD_9 97
#define UKEY_PAD_0 98

#define UKEY_PAD_SEPARATOR 99

#define UKEY_BACKSLASH_NON_US 100
#define UKEY_APPLICATION 101
#define UKEY_POWER 102

#define UKEY_MUTE 127
#define UKEY_VOLUME_UP 128
#define UKEY_VOLUME_DOWN 129

#define UKEY_LEFT_CONTROL 224
#define UKEY_LEFT_SHIFT 225
#define UKEY_LEFT_ALT 226
#define UKEY_LEFT_GUI 227
#define UKEY_RIGHT_CONTROL 228
#define UKEY_RIGHT_SHIFT 229
#define UKEY_RIGHT_ALT 230
#define UKEY_RIGHT_GUI 231

#define UKEY_BIT_LEFT_CONTROL (1 << 0)
#define UKEY_BIT_LEFT_SHIFT (1 << 1)
#define UKEY_BIT_LEFT_ALT (1 << 2)
#define UKEY_BIT_LEFT_GUI (1 << 3)
#define UKEY_BIT_RIGHT_CONTROL (1 << 4)
#define UKEY_BIT_RIGHT_SHIFT (1 << 5)
#define UKEY_BIT_RIGHT_ALT (1 << 6)
#define UKEY_BIT_RIGHT_GUI (1 << 7)

typedef uint8_t Layout_t[ROWS][COLS];

//uint8_t get_row(uint16_t);
void get_rows(uint8_t[ROWS], const uint16_t[ROWS]);

//uint8_t lyt_modmask(const uint8_t);
//uint8_t lyt_is_modkey(const uint8_t);
uint8_t lyt_get_pressed(const Layout_t *, uint8_t[NSWITCH], uint8_t *, const uint8_t[ROWS]);
void lyt_select_layout(const Layout_t **, const uint16_t (**)[ROWS]);

#include "bits.h"
uint8_t lyt_report_bits(const Layout_t *layout, Bits_t bs, const uint8_t rows[ROWS]);

#endif
