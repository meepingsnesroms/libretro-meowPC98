/**
 * @file	sdlkey.c
 * @brief	Implementation of the keyboard
 */

#include "compiler.h"
#include "np2.h"
#include "sdlkbd.h"
#include "keystat.h"
#include "libretro.h"

typedef struct {
	int sdlkey;
	UINT8 keycode;
} SDLKCNV;

#define		NC		0xff

/*! 101 keyboard key table */
static const SDLKCNV sdlcnv101[] =
{
			{RETROK_ESCAPE,		0x00},	{RETROK_1,			0x01},
			{RETROK_2,			0x02},	{RETROK_3,			0x03},
			{RETROK_4,			0x04},	{RETROK_5,			0x05},
			{RETROK_6,			0x06},	{RETROK_7,			0x07},

			{RETROK_8,			0x08},	{RETROK_9,			0x09},
			{RETROK_0,			0x0a},	{RETROK_MINUS,		0x0b},
			{RETROK_CARET,		0x0c},	{RETROK_BACKSLASH,	0x0d},
			{RETROK_BACKSPACE,	0x0e},	{RETROK_TAB,			0x0f},

			{RETROK_q,			0x10},	{RETROK_w,			0x11},
			{RETROK_e,			0x12},	{RETROK_r,			0x13},
			{RETROK_t,			0x14},	{RETROK_y,			0x15},
			{RETROK_u,			0x16},	{RETROK_i,			0x17},

			{RETROK_o,			0x18},	{RETROK_p,			0x19},
			{RETROK_AT,			0x1a},	{RETROK_LEFTBRACKET,	0x1b},
			{RETROK_RETURN,		0x1c},	{RETROK_a,			0x1d},
			{RETROK_s,			0x1e},	{RETROK_d,			0x1f},

			{RETROK_f,			0x20},	{RETROK_g,			0x21},
			{RETROK_h,			0x22},	{RETROK_j,			0x23},
			{RETROK_k,			0x24},	{RETROK_l,			0x25},
			{RETROK_SEMICOLON,	0x26},	{RETROK_COLON,		0x27},

			{RETROK_RIGHTBRACKET,	0x28},	{RETROK_z,			0x29},
			{RETROK_x,			0x2a},	{RETROK_c,			0x2b},
			{RETROK_v,			0x2c},	{RETROK_b,			0x2d},
			{RETROK_n,			0x2e},	{RETROK_m,			0x2f},

			{RETROK_COMMA,		0x30},	{RETROK_PERIOD,		0x31},
			{RETROK_SLASH,		0x32},	{RETROK_UNDERSCORE,	0x33},
			{RETROK_SPACE,		0x34},
			{RETROK_PAGEUP,		0x36},	{RETROK_PAGEDOWN,		0x37},

			{RETROK_INSERT,		0x38},	{RETROK_DELETE,		0x39},
			{RETROK_UP,			0x3a},	{RETROK_LEFT,			0x3b},
			{RETROK_RIGHT,		0x3c},	{RETROK_DOWN,			0x3d},
			{RETROK_HOME,			0x3e},	{RETROK_END,			0x3f},

			{RETROK_KP_MINUS,		0x40},	{RETROK_KP_DIVIDE,	0x41},
			{RETROK_KP7,			0x42},	{RETROK_KP8,			0x43},
			{RETROK_KP9,			0x44},	{RETROK_KP_MULTIPLY,	0x45},
			{RETROK_KP4,			0x46},	{RETROK_KP5,			0x47},

			{RETROK_KP6,			0x48},	{RETROK_KP_PLUS,		0x49},
			{RETROK_KP1,			0x4a},	{RETROK_KP2,			0x4b},
			{RETROK_KP3,			0x4c},	{RETROK_KP_EQUALS,	0x4d},
			{RETROK_KP0,			0x4e},

			{RETROK_KP_PERIOD,	0x50},

			{RETROK_PAUSE,		0x60},	{RETROK_PRINT,	0x61},
			{RETROK_F1,			0x62},	{RETROK_F2,			0x63},
			{RETROK_F3,			0x64},	{RETROK_F4,			0x65},
			{RETROK_F5,			0x66},	{RETROK_F6,			0x67},

			{RETROK_F7,			0x68},	{RETROK_F8,			0x69},
			{RETROK_F9,			0x6a},	{RETROK_F10,			0x6b},

			{RETROK_RSHIFT,		0x70},	{RETROK_LSHIFT,		0x70},
			{RETROK_CAPSLOCK,		0x71},
			{RETROK_RALT,			0x73},	{RETROK_LALT,			0x73},
			{RETROK_RCTRL,		0x74},	{RETROK_LCTRL,		0x74},

			/* = */
			{RETROK_EQUALS,		0x0c},

			/* MacOS Yen */
			{0xa5,				0x0d},
};

/*! extend key */
static const UINT8 f12keys[] = {0x61, 0x60, 0x4d, 0x4f};

/**
 * Initialize
 */
void sdlkbd_initialize(void)
{
}

/**
 * Serializes
 * @param[in] key Key code
 * @return PC-98 data
 */
static UINT8 getKey(int key)
{
	size_t i;

	for (i = 0; i < SDL_arraysize(sdlcnv101); i++)
	{
		if (sdlcnv101[i].sdlkey == key)
		{
			return sdlcnv101[i].keycode;
		}
	}
	return NC;
}

/**
 * Get F12 settings
 * @return PC-98 data
 */
static UINT8 getf12key(void)
{
	UINT	key;

	key = np2oscfg.F12KEY - 1;
	if (key < SDL_arraysize(f12keys))
	{
		return f12keys[key];
	}
	else
	{
		return NC;
	}
}

/**
 * Key down
 * @param[in] key Key code
 */
void sdlkbd_keydown(UINT key)
{
	UINT8	data;

	if (key == RETROK_F12)
	{
		data = getf12key();
	}
	else
	{
		data = getKey(key);
	}
	if (data != NC)
	{
		keystat_senddata(data);
	}
}

/**
 * Key up
 * @param[in] key Key code
 */
void sdlkbd_keyup(UINT key)
{
	UINT8	data;

	if (key == RETROK_F12)
	{
		data = getf12key();
	}
	else
	{
		data = getKey(key);
	}
	if (data != NC)
	{
		keystat_senddata((UINT8)(data | 0x80));
	}
}

/**
 * Reset F12
 */
void sdlkbd_resetf12(void)
{
	size_t i;

	for (i = 0; i < SDL_arraysize(f12keys); i++)
	{
		keystat_forcerelease(f12keys[i]);
	}
}
