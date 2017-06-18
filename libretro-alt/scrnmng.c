#include	"compiler.h"
// #include	<sys/time.h>
// #include	<signal.h>
// #include	<unistd.h>
#include	"scrnmng.h"
#include	"scrndraw.h"
#include	"vramhdl.h"
#include	"menubase.h"

extern unsigned short int videoBuffer[640*400];
extern unsigned short int videoBuffer2[640*400];
/*
static SDL_Window *s_sdlWindow;
static SDL_Renderer *s_renderer;
static SDL_Texture *s_texture;
static SDL_Surface *s_surface;
*/

typedef struct {
	BOOL		enable;
	int			width;
	int			height;
	int			bpp;
	int	*surface;
	VRAMHDL		vram;
} SCRNMNG;

typedef struct {
	int		width;
	int		height;
} SCRNSTAT;

static const char app_name[] = "Neko Project II";

static	SCRNMNG		scrnmng;
static	SCRNSTAT	scrnstat;
static	SCRNSURF	scrnsurf;

typedef struct {
	int		xalign;
	int		yalign;
	int		width;
	int		height;
	int		srcpos;
	int		dstpos;
} DRAWRECT;

static BRESULT calcdrawrect(int *surface,
								DRAWRECT *dr, VRAMHDL s, const RECT_T *rt) {

	int		pos;

	dr->xalign = 2;
	dr->yalign = 640*2;
	dr->srcpos = 0;
	dr->dstpos = 0;
	dr->width = min(scrnmng.width, s->width);
	dr->height = min(scrnmng.height, s->height);
	if (rt) {
		pos = max(rt->left, 0);
		dr->srcpos += pos;
		dr->dstpos += pos * dr->xalign;
		dr->width = min(rt->right, dr->width) - pos;

		pos = max(rt->top, 0);
		dr->srcpos += pos * s->width;
		dr->dstpos += pos * dr->yalign;
		dr->height = min(rt->bottom, dr->height) - pos;
	}
	if ((dr->width <= 0) || (dr->height <= 0)) {
		return(FAILURE);
	}
	return(SUCCESS);
}


void scrnmng_initialize(void) {

	scrnstat.width = 640;
	scrnstat.height = 400;
}

BRESULT scrnmng_create(int width, int height) {

	scrnmng.enable = TRUE;
	scrnmng.width = width;
	scrnmng.height = height;
	scrnmng.bpp = 16;
printf("wh(%d,%d)\n",width, height);
	return(SUCCESS);

}

void scrnmng_destroy(void) {

	scrnmng.enable = FALSE;
}

RGB16 scrnmng_makepal16(RGB32 pal32) {

	RGB16	ret;

	ret = (pal32.p.r & 0xf8) << 8;
#if defined(SIZE_QVGA)
	ret += (pal32.p.g & 0xfc) << (3 + 16);
#else
	ret += (pal32.p.g & 0xfc) << 3;
#endif
	ret += pal32.p.b >> 3;
	return(ret);
}

void scrnmng_setwidth(int posx, int width) {

	scrnstat.width = width;
}

void scrnmng_setheight(int posy, int height) {

	scrnstat.height = height;
}

const SCRNSURF *scrnmng_surflock(void) {


	if (scrnmng.vram == NULL) {

		scrnmng.surface = NULL;
		scrnsurf.ptr = (BYTE *)videoBuffer;
		scrnsurf.xalign = 2;
		scrnsurf.yalign = 640*2;
		scrnsurf.bpp = 16;
	}
	else {
		scrnsurf.ptr = scrnmng.vram->ptr;
		scrnsurf.xalign = scrnmng.vram->xalign;
		scrnsurf.yalign = scrnmng.vram->yalign;
		scrnsurf.bpp = scrnmng.vram->bpp;
	}
	scrnsurf.width = min(scrnstat.width, 640);
	scrnsurf.height = min(scrnstat.height, 400);
	scrnsurf.extend = 0;
	return(&scrnsurf);
}

static void draw_onmenu(void) {

	RECT_T		rt;
	int		*surface;
	DRAWRECT	dr;
const UINT8		*p;
	UINT8		*q;
const UINT8		*a;
	int			salign;
	int			dalign;
	int			x;

	rt.left = 0;
	rt.top = 0;
	rt.right = min(scrnstat.width, 640);
	rt.bottom = min(scrnstat.height, 400);
#if defined(SIZE_QVGA)
	rt.right >>= 1;
	rt.bottom >>= 1;
#endif
/*
	surface = s_surface;
	if (surface == NULL) {
		return;
	}
	SDL_LockSurface(surface);
*/
	if (calcdrawrect(NULL, &dr, menuvram, &rt) == SUCCESS) {
		switch(scrnmng.bpp) {
#if defined(SUPPORT_16BPP)
			case 16:
				p = scrnmng.vram->ptr + (dr.srcpos * 2);
				q = (BYTE *)videoBuffer2 + dr.dstpos;
				a = menuvram->alpha + dr.srcpos;
				salign = menuvram->width;
				dalign = dr.yalign - (dr.width * dr.xalign);
				do {
					x = 0;
					do {
						if (a[x] == 0) {
							*(UINT16 *)q = *(UINT16 *)(p + (x * 2));
						}
						q += dr.xalign;
					} while(++x < dr.width);
					p += salign * 2;
					q += dalign;
					a += salign;
				} while(--dr.height);
				break;
#endif
		}
	}
}

void scrnmng_surfunlock(const SCRNSURF *surf) {

	//SDL_Surface	*surface;

	if (surf) {
		if (scrnmng.vram == NULL) {
/*
			if (scrnmng.surface != NULL) {
				surface = scrnmng.surface;
				scrnmng.surface = NULL;
			}
*/
		}
		else {
			if (menuvram) {
				draw_onmenu();
			}
		}
	}
}


// ----

BRESULT scrnmng_entermenu(SCRNMENU *smenu) {

	if (smenu == NULL) {
		goto smem_err;
	}
	vram_destroy(scrnmng.vram);
	scrnmng.vram = vram_create(scrnmng.width, scrnmng.height, FALSE,
																scrnmng.bpp);
	if (scrnmng.vram == NULL) {
		goto smem_err;
	}
	scrndraw_redraw();
	smenu->width = scrnmng.width;
	smenu->height = scrnmng.height;
	smenu->bpp = (scrnmng.bpp == 32)?24:scrnmng.bpp;
	return(SUCCESS);

smem_err:
	return(FAILURE);
}

void scrnmng_leavemenu(void) {

	VRAM_RELEASE(scrnmng.vram);
}

void scrnmng_menudraw(const RECT_T *rct) {

	//SDL_Surface	*surface;
	DRAWRECT	dr;
const UINT8		*p;
const UINT8		*q;
	UINT8		*r;
	UINT8		*a;
	int			salign;
	int			dalign;
	int			x;

	if ((!scrnmng.enable) && (menuvram == NULL)) {
		return;
	}
/*
	surface = s_surface;
	if (surface == NULL) {
		return;
	}
	SDL_LockSurface(surface);
*/
	if (calcdrawrect(NULL, &dr, menuvram, rct) == SUCCESS) {
		switch(scrnmng.bpp) {
#if defined(SUPPORT_16BPP)
			case 16:
				p = scrnmng.vram->ptr + (dr.srcpos * 2);
				q = menuvram->ptr + (dr.srcpos * 2);
				r = (BYTE *)videoBuffer2 + dr.dstpos;
				a = menuvram->alpha + dr.srcpos;
				salign = menuvram->width;
				dalign = dr.yalign - (dr.width * dr.xalign);
				do {
					x = 0;
					do {
						if (a[x]) {
							if (a[x] & 2) {
								*(UINT16 *)r = *(UINT16 *)(q + (x * 2));
							}
							else {
								a[x] = 0;
								*(UINT16 *)r = *(UINT16 *)(p + (x * 2));
							}
						}
						r += dr.xalign;
					} while(++x < dr.width);
					p += salign * 2;
					q += salign * 2;
					r += dalign;
					a += salign;
				} while(--dr.height);
				break;
#endif

		}
	}

}

