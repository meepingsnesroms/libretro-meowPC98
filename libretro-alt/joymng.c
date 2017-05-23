#include	"compiler.h"
#include	"joymng.h"

#include "libretro.h"

extern retro_input_state_t input_state_cb;

static UINT8 s_cJoyFlag = 0;

enum
{
	JOY_LEFT_BIT	= 0x04,
	JOY_RIGHT_BIT	= 0x08,
	JOY_UP_BIT	= 0x01,
	JOY_DOWN_BIT	= 0x02,
	JOY_BTN1_BIT	= 0x10,
	JOY_BTN2_BIT	= 0x20,
	JOY_BTN3_BIT	= 0x40,
	JOY_BTN4_BIT	= 0x80
};

void joymng_sync()
{
	s_cJoyFlag = 0;
}

BYTE joymng_getstat(void) {

	if (s_cJoyFlag == 0)
	{
		UINT8 cJoyFlag = 0xff;

	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP))cJoyFlag &= ~JOY_UP_BIT;
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN))cJoyFlag &= ~JOY_DOWN_BIT;
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT))cJoyFlag &= ~JOY_LEFT_BIT;
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT))cJoyFlag &= ~JOY_RIGHT_BIT;

	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A) )cJoyFlag &= ~JOY_BTN1_BIT;
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B) )cJoyFlag &= ~JOY_BTN2_BIT;
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X) )cJoyFlag &= ~JOY_BTN3_BIT;
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y) )cJoyFlag &= ~JOY_BTN4_BIT;

	s_cJoyFlag = cJoyFlag;

	}
	return s_cJoyFlag;

}

