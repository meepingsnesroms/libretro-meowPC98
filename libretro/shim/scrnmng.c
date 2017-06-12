#include	"compiler.h"
#include	"scrnmng.h"
#include	"scrndraw.h"
#include	"vramhdl.h"
#include	"menubase.h"

#include "libretro_exports.h"

typedef struct {
	int		xalign;
	int		yalign;
	int		width;
	int		height;
	int		srcpos;
	int		dstpos;
} DRAWRECT;

static	SCRNSURF	scrnsurf;
static	VRAMHDL		vram;

void scrnmng_initialize(void){
	scrnsurf.width = 640;
	scrnsurf.height =  400;
}

void scrnmng_destroy(void){}

const SCRNSURF *scrnmng_surflock(void){

	scrnsurf.width = 640;
	scrnsurf.height =  400;
	scrnsurf.xalign = 2;
	scrnsurf.yalign = 640*2;
	scrnsurf.bpp = 16;
	scrnsurf.extend = 0;

	if (vram == NULL)
		scrnsurf.ptr = (BYTE *)FrameBuffer;
	else 
		scrnsurf.ptr = vram->ptr;

	return(&scrnsurf);
}

static BRESULT calcdrawrect(DRAWRECT *dr, const RECT_T *rt) {

	int		pos;

	dr->xalign = 2;
	dr->yalign = 640*2;
	dr->srcpos = 0;
	dr->dstpos = 0;
	dr->width = 640;
	dr->height = 400;
	if (rt) {
		pos = max(rt->left, 0);
		dr->srcpos += pos;
		dr->dstpos += pos * dr->xalign;
		dr->width = min(rt->right, dr->width) - pos;

		pos = max(rt->top, 0);
		dr->srcpos += pos * 640;
		dr->dstpos += pos * dr->yalign;
		dr->height = min(rt->bottom, dr->height) - pos;
	}
	if ((dr->width <= 0) || (dr->height <= 0)) {
		return(FAILURE);
	}
	return(SUCCESS);
}

void draw(DRAWRECT dr){

	const UINT8		*p;
	const UINT8		*q;
	UINT8		*r;
	UINT8		*a;
	int			salign;
	int			dalign;
	int			x;

	p = vram->ptr + (dr.srcpos * 2);
	q = (BYTE *)GuiBuffer + dr.dstpos;
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
}

void draw2(DRAWRECT dr){

	const UINT8		*p;
	const UINT8		*q;
	UINT8		*r;
	UINT8		*a;
	int			salign;
	int			dalign;
	int			x;

	p = vram->ptr + (dr.srcpos * 2);
	q = menuvram->ptr + (dr.srcpos * 2);
	r = (BYTE *)GuiBuffer + dr.dstpos;
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

}


static void draw_onmenu(void) {

	RECT_T		rt;
	DRAWRECT	dr;

	rt.left = 0;
	rt.top = 0;
	rt.right =  640;
	rt.bottom = 400;

	if (calcdrawrect( &dr, &rt) == SUCCESS)
		draw(dr);

}

void scrnmng_surfunlock(const SCRNSURF *surf){

	if (surf)
		if (vram == NULL);
		else{
			if (menuvram)
				draw_onmenu();			
		}

}

BRESULT scrnmng_create(int width, int height) {

   if(width != 640 || height != 400){
      printf("Invalid screen size:%dx%d\n", width, height);
      abort();
   }
   
   scrnsurf.ptr    = (UINT8*)FrameBuffer;
   scrnsurf.xalign = 2;//bytes per pixel
   scrnsurf.yalign = 640 * 2;//bytes per line
   scrnsurf.width  = width;
   scrnsurf.height = height;
   scrnsurf.bpp    = 16;
   scrnsurf.extend = 0;//?
   return(SUCCESS);
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

   if(width != 640){
      printf("Set invalid screen width.\n");
      abort();
   }
   
	scrnsurf.width = width;
}

void scrnmng_setheight(int posy, int height) {

   if(height != 400){
      printf("Set invalid screen height.\n");
      abort();
   }
   
	scrnsurf.height = height;
}

// ----

BRESULT scrnmng_entermenu(SCRNMENU *smenu) {

	if (smenu == NULL) {
		goto smem_err;
	}
	vram_destroy(vram);
	vram = vram_create(640,400,FALSE,16);

	if (vram == NULL) {
		goto smem_err;
	}
	scrndraw_redraw();
	smenu->width = 640;
	smenu->height = 400;
	smenu->bpp = 16;

	return(SUCCESS);
smem_err:
	return(FAILURE);
}

void scrnmng_leavemenu(void) {

	VRAM_RELEASE(vram);
}

void scrnmng_menudraw(const RECT_T *rct) {

	DRAWRECT	dr;

	//if (menuvram == NULL) 
	//	return;
	if (calcdrawrect( &dr, rct) == SUCCESS) 
		draw2(dr);
}

