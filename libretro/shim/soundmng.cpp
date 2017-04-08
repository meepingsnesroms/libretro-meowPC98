#include "compiler.h"
#include "soundmng.h"
#include <algorithm>
#include "parts.h"
#include "sound.h"
#if defined(VERMOUTH_LIB)
#include "commng.h"
#include "cmver.h"
#endif
#if defined(SUPPORT_EXTERNALCHIP)
#include "ext/externalchipmanager.h"
#endif

SOUNDMNG	soundmng;

UINT soundmng_create(UINT rate, UINT ms) {
   
   UINT			s;
   UINT			samples;
   
   if (rate != 16000) abort();
   
   if (soundmng.opened) {
      return(0);
   }

   
   s = rate * ms / (NSNDBUF * 1000);
   samples = 1;
   while(s > samples) {
      samples <<= 1;
   }
   
   soundmng.buf = (SINT16 *)_MALLOC(samples * 2 * sizeof(SINT16), "buf");
   if (!soundmng.buf) {
      return(0);
   }
   ZeroMemory(soundmng.buf, samples * 2 * sizeof(SINT16));
   
   soundmng.nsndbuf = 0;
   soundmng.samples = samples;
   
#if defined(VERMOUTH_LIB)
   cmvermouth_load(rate);
#endif
   
   soundmng.opened = TRUE;
   return(samples);
}

void soundmng_destroy(void) {
   
   if (soundmng.opened) {
      soundmng.opened = FALSE;
      _MFREE(soundmng.buff);
   }
}

void soundmng_play(void)
{
	if (soundmng.opened)
	{
      soundmng.enabled = TRUE;
#if defined(SUPPORT_EXTERNALCHIP)
		CExternalChipManager::GetInstance()->Mute(false);
#endif
	}
}

void soundmng_stop(void)
{
	if (soundmng.opened)
	{
      soundmng.enabled = FALSE;
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
