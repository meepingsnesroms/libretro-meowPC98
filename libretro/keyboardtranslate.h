#ifndef KBLUT_
#define KBLUT_

#include <stdint.h>

extern uint16_t keys_to_poll[];
extern uint16_t keys_needed;

void init_lr_key_to_pc98();
void send_libretro_key_down(uint16_t key);
void send_libretro_key_up(uint16_t key);
void sdlkbd_resetf12(void);
#endif
