#include	"compiler.h"
#include	"inputmng.h"
#include	"libretro.h"

typedef struct {
	int	key;
	UINT		bit;
} KEYBIND;

typedef struct {
	UINT	kbs;
	KEYBIND	kb[32];
} INPMNG;

static	INPMNG	inpmng;

static const KEYBIND keybind[] = {
					{RETROK_UP,		KEY_UP},
					{RETROK_DOWN,		KEY_DOWN},
					{RETROK_LEFT,		KEY_LEFT},
					{RETROK_RIGHT,	KEY_RIGHT},
					{RETROK_RETURN,	KEY_ENTER},
					{RETROK_ESCAPE,	KEY_MENU},
					{RETROK_TAB,		KEY_SKIP}};		/* ‚Æ‚è‚ ‚¦‚¸‚Ëc */


// ----

void inputmng_init(void) {

	INPMNG	*im;

	im = &inpmng;
	ZeroMemory(im, sizeof(INPMNG));
	im->kbs = sizeof(keybind) / sizeof(KEYBIND);
	CopyMemory(im->kb, keybind, sizeof(keybind));
}

void inputmng_keybind(short key, UINT bit) {

	INPMNG	*im;
	UINT	i;

	im = &inpmng;
	for (i=0; i<im->kbs; i++) {
		if (im->kb[i].key == key) {
			im->kb[i].bit = bit;
			return;
		}
	}
	if (im->kbs < (sizeof(im->kb) / sizeof(KEYBIND))) {
		im->kb[im->kbs].key = key;
		im->kb[im->kbs].bit = bit;
		im->kbs++;
	}
}

UINT inputmng_getkey(short key) {

	INPMNG	*im;
	KEYBIND	*kb;
	UINT	kbs;

	im = &inpmng;
	kb = im->kb;
	kbs = im->kbs;
	while(kbs--) {
		if (kb->key == key) {
			return(kb->bit);
		}
		kb++;
	}
	return(0);
}

