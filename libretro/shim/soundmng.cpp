#include "compiler.h"
#include "soundmng.h"
#include "parts.h"
#include "sound.h"
#if defined(VERMOUTH_LIB)
#include "commng.h"
#include "cmver.h"
#endif
#if defined(SUPPORT_EXTERNALCHIP)
#include "ext/externalchipmanager.h"
#endif
#include "libretro_exports.h"
typedef struct {
	BOOL     opened;
} SOUNDMNG;

#define NSNDBUF 1

static	SOUNDMNG	soundmng;
extern "C" {
void playretro(){
	const SINT32	*src;
	int length=735*4;
	SINT16		dst[735*2];
	src = sound_pcmlock();

	if (src) {
		satuation_s16(dst, src, length);
		sound_pcmunlock(src);
	}
	else {
		ZeroMemory(dst, length);
	}
	audio_batch_cb(dst,length/4);
}
}
UINT soundmng_create(UINT rate, UINT ms) {
   if(rate != 44100){
      printf("Invalid uddio rate:%d Moo\n", rate);
      abort();
   }
   if(ms != 0){
      printf("Invalid latency setting:%d\n", ms);
      abort();
   }
   
   //UINT s = rate * ms / (NSNDBUF * 1000);
   //UINT s = (rate / 60) * ms;
   UINT s = rate / 60;
   UINT samples = rate ;
   while(s > samples) {
      samples <<= 1;
   }
   
   printf("Samples:%d\n", samples);
   
#if defined(VERMOUTH_LIB)
   cmvermouth_load(rate);
#endif
	return(samples);
}

void soundmng_destroy(void)
{
   if (soundmng.opened) {
      soundmng.opened = FALSE;
   }
}

void soundmng_play(void)
{
   if (soundmng.opened)
   {
#if defined(SUPPORT_EXTERNALCHIP)
      CExternalChipManager::GetInstance()->Mute(false);
#endif
   }
}

void soundmng_stop(void)
{
   if (soundmng.opened)
   {
#if defined(SUPPORT_EXTERNALCHIP)
      CExternalChipManager::GetInstance()->Mute(true);
#endif
   }
}


// ----

void soundmng_initialize()
{
#if defined(SUPPORT_EXTERNALCHIP)
	CExternalChipManager::GetInstance()->Initialize();
#endif
}

void soundmng_deinitialize()
{
#if defined(SUPPORT_EXTERNALCHIP)
	CExternalChipManager::GetInstance()->Deinitialize();
#endif

#if defined(VERMOUTH_LIB)
	cmvermouth_unload();
#endif
}
