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

#define	NSNDBUF				2

extern signed short soundbuf[1024*2];
#include "libretro.h"
extern retro_audio_sample_batch_t audio_batch_cb;

typedef struct {
	BOOL	opened;
	int		nsndbuf;
	int		samples;
	SINT16	*buf[NSNDBUF];
} SOUNDMNG;

static	SOUNDMNG	soundmng;
extern "C" {
 void sound_play_cb(void *userdata, UINT8 *stream, int len) {

	int			length;
	SINT16		*dst;
const SINT32	*src;

	length = min(len, (int)(soundmng.samples * 2 * sizeof(SINT16)));
	dst = soundbuf;//soundmng.buf[soundmng.nsndbuf];
	src = sound_pcmlock();
	if (src) {
		satuation_s16(dst, src, length);
		sound_pcmunlock(src);
	}
	else {
		ZeroMemory(dst, length);
	}
	//memset(stream, 0, len);
	//memset(soundbuf,len);
   	audio_batch_cb(soundbuf,len/4);
//	SDL_MixAudio(stream, (UINT8 *)dst, length, SDL_MIX_MAXVOLUME);
	soundmng.nsndbuf = (soundmng.nsndbuf + 1) % NSNDBUF;
	(void)userdata;
}
}

UINT soundmng_create(UINT rate, UINT ms) {

   if(rate != 44100){
      printf("Invalid audio rate:%d Moo\n", rate);
      abort();
   }
/*
   if(ms == 0){
      printf("Default ms used ms:%d \n", ms);
      ms=250;
   }
*/
	UINT	s;
	UINT	samples;

	if (soundmng.opened) {
		goto smcre_err1;
	}
/*
	s = rate * ms / (NSNDBUF * 1000);
	samples = 1;
	while(s > samples) {
		samples <<= 1;
	}
	soundmng.nsndbuf = 0;
	soundmng.samples = samples;
*/
	soundmng.nsndbuf = 0;
	soundmng.samples = samples = 1024;

	soundmng.opened = TRUE;
   
   printf("Samples:%d\n", samples);
   
#if defined(VERMOUTH_LIB)
   cmvermouth_load(rate);
#endif

	return(samples);
/*
smcre_err2:
	for (s=0; s<NSNDBUF; s++) {
		tmp = soundmng.buf[s];
		soundmng.buf[s] = NULL;
		if (tmp) {
			_MFREE(tmp);
		}
	}
*/
smcre_err1:
	return(0);

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
