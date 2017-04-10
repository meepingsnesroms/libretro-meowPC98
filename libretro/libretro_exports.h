#ifndef LREXPORTS_
#define LREXPORTS_

#include <stdint.h>

extern uint16_t   FrameBuffer[];
extern void*      audio_buffer;
extern uint32_t   audio_samples;
extern bool       audio_paused;
#endif
