
enum {
	uPD8255A_LEFTBIT	= 0x80,
	uPD8255A_RIGHTBIT	= 0x20
};


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	SINT16	x;
	SINT16	y;
	UINT8	btn;
	UINT	flag;
} MOUSEMNG;

extern MOUSEMNG	mousemng;

UINT8 mousemng_getstat(SINT16 *x, SINT16 *y, int clear);

#ifdef __cplusplus
}
#endif

// ---- for libretro

enum {
	MOUSEMNG_LEFTDOWN		= 0,
	MOUSEMNG_LEFTUP,
	MOUSEMNG_RIGHTDOWN,
	MOUSEMNG_RIGHTUP
};

enum {
	MOUSEPROC_SYSTEM		= 0,
	MOUSEPROC_WINUI,
	MOUSEPROC_BG
};


void mousemng_initialize(void);
void mousemng_sync(int mpx,int mpy);
BOOL mousemng_buttonevent(UINT event);
void mousemng_enable(UINT proc);
void mousemng_disable(UINT proc);
void mousemng_toggle(UINT proc);
