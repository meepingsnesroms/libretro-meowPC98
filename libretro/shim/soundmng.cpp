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

#define	NSNDBUF				2

typedef struct {
	BOOL	opened;
	int		nsndbuf;
	int		samples;
	SINT16	*buf[NSNDBUF];
} SOUNDMNG;

static	SOUNDMNG	soundmng;

UINT soundmng_create(UINT rate, UINT ms) {
	return(0);
}

void soundmng_destroy(void)
{
}

void soundmng_play(void)
{
}

void soundmng_stop(void)
{
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
