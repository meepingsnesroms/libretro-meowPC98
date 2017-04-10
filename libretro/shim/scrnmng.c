#include	"compiler.h"
#include	"scrnmng.h"
#include	"scrndraw.h"
#include	"vramhdl.h"
#include	"menubase.h"

#include "libretro_exports.h"

static	SCRNSURF	scrnsurf;

void scrnmng_initialize(void){}
void scrnmng_destroy(void){}
const SCRNSURF *scrnmng_surflock(void){return(&scrnsurf);}
void scrnmng_surfunlock(const SCRNSURF *surf){}

BRESULT scrnmng_create(int width, int height) {

   if(width != 640 || height != 400){
      printf("Invalid screen size.\n");
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
	return(FAILURE);
}

void scrnmng_leavemenu(void) {
   //no menu, use retroarch settings menu
}

void scrnmng_menudraw(const RECT_T *rct) {
   //no menu, use retroarch settings menu
}

