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

#include "libretro_exports.h"

typedef struct {
	BOOL     opened;
} SOUNDMNG;

static	SOUNDMNG	soundmng;

UINT soundmng_create(UINT rate, UINT ms) {
   if(rate != 44100){
      printf("Invalid uddio rate:%d Moo\n", rate);
      abort();
   }
   
   audio_paused = false;
#if defined(VERMOUTH_LIB)
   cmvermouth_load(rate);
#endif
	return(44100);
}

void soundmng_destroy(void)
{
   if (soundmng.opened) {
      soundmng.opened = FALSE;
      audio_paused = true;
   }
}

void soundmng_play(void)
{
   if (soundmng.opened)
   {
      audio_paused = false;
#if defined(SUPPORT_EXTERNALCHIP)
      CExternalChipManager::GetInstance()->Mute(false);
#endif
   }
}

void soundmng_stop(void)
{
   if (soundmng.opened)
   {
      audio_paused = true;
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
