#include "compiler.h"
#include	"strres.h"
#include	"np2.h"
#include	"dosio.h"
#include	"commng.h"
#include	"fontmng.h"
#include	"inputmng.h"
#include	"scrnmng.h"
#include	"soundmng.h"
#include	"sysmng.h"
#include	"taskmng.h"
#include	"sdlkbd.h"
#include	"ini.h"
#include	"pccore.h"
#include	"statsave.h"
#include	"iocore.h"
#include	"scrndraw.h"
#include	"s98.h"
#include	"fdd/diskdrv.h"
#include	"timing.h"
#include	"keystat.h"
#include	"vramhdl.h"
#include	"menubase.h"
#include	"sysmenu.h"


NP2OSCFG	np2oscfg = {0, 0, 0, 0, 0};
static	UINT		framecnt;
static	UINT		waitcnt;
static	UINT		framemax = 1;

static void usage(const char *progname) {

	printf("Usage: %s [options]\n", progname);
	printf("\t--help   [-h]       : print this message\n");
}


// ---- resume

static void getstatfilename(char *path, const char *ext, int size)
{
	char filename[32];
	sprintf(filename, "np2sdl2.%s", ext);

	file_cpyname(path, file_getcd(filename), size);
}

static int flagsave(const char *ext) {

	int		ret;
	char	path[MAX_PATH];

	getstatfilename(path, ext, sizeof(path));
	ret = statsave_save(path);
	if (ret) {
		file_delete(path);
	}
	return(ret);
}

static void flagdelete(const char *ext) {

	char	path[MAX_PATH];

	getstatfilename(path, ext, sizeof(path));
	file_delete(path);
}

static int flagload(const char *ext, const char *title, BOOL force) {

	int		ret;
	int		id;
	char	path[MAX_PATH];
	char	buf[1024];
	char	buf2[1024 + 256];

	getstatfilename(path, ext, sizeof(path));
	id = DID_YES;
	ret = statsave_check(path, buf, sizeof(buf));
	if (ret & (~STATFLAG_DISKCHG)) {
		menumbox("Couldn't restart", title, MBOX_OK | MBOX_ICONSTOP);
		id = DID_NO;
	}
	else if ((!force) && (ret & STATFLAG_DISKCHG)) {
		SPRINTF(buf2, "Conflict!\n\n%s\nContinue?", buf);
		id = menumbox(buf2, title, MBOX_YESNOCAN | MBOX_ICONQUESTION);
	}
	if (id == DID_YES) {
		statsave_load(path);
	}
	return(id);
}


// ---- proc

#define	framereset(cnt)		framecnt = 0

static void processwait(UINT cnt) {

	if (timing_getcount() >= cnt) {
		timing_setcount(0);
		framereset(cnt);
	}
	else {
		taskmng_sleep(1);
	}
}

char* get_file_ext(char* filepath){
   if(filepath[strlen(filepath) - 2]=='.')return filepath + strlen(filepath) - 2;
   return filepath + strlen(filepath) - 3;
}

//d88 88d d98 98d fdi fdd xdf hdm dup 2hd tfd
//thd nhd hdi hdd

int np2_main(int argc, char *argv[]) {

	int		pos;
	char	*p;
	int		id;
       
	int loaddisk[8]={-1,-1,-1,-1,-1,-1,-1,-1};

	pos = 1;
	while(pos < argc) {
		p = argv[pos++];
		if ((!milstr_cmp(p, "-h")) || (!milstr_cmp(p, "--help"))) {
			usage(argv[0]);
			goto np2main_err1;
		}
		else if(strcmp(get_file_ext(p), "thd") == 0 ||\
			strcmp(get_file_ext(p), "THD") == 0 ||\
			strcmp(get_file_ext(p), "nhd") == 0 ||\
			strcmp(get_file_ext(p), "NHD") == 0 ||\
			strcmp(get_file_ext(p), "hdi") == 0 ||\
			strcmp(get_file_ext(p), "HDI") == 0 ||\
			strcmp(get_file_ext(p), "hdd") == 0 ||\
			strcmp(get_file_ext(p), "HDD") == 0  )
	        {
			printf("POS:%d (%s)\n",pos-2,p);
			loaddisk[pos-2]=4+pos-2; //break;
		}
		else if(strcmp(get_file_ext(p), "88d") == 0 ||\
			strcmp(get_file_ext(p), "88D") == 0 ||\
			strcmp(get_file_ext(p), "d98") == 0 ||\
			strcmp(get_file_ext(p), "D98") == 0 ||\
			strcmp(get_file_ext(p), "98d") == 0 ||\
			strcmp(get_file_ext(p), "98D") == 0 ||\
			strcmp(get_file_ext(p), "fdi") == 0 ||\
			strcmp(get_file_ext(p), "FDI") == 0 ||\
			strcmp(get_file_ext(p), "fdd") == 0 ||\
			strcmp(get_file_ext(p), "FDD") == 0 ||\
			strcmp(get_file_ext(p), "xdf") == 0 ||\
			strcmp(get_file_ext(p), "XDF") == 0 ||\
			strcmp(get_file_ext(p), "hdm") == 0 ||\
			strcmp(get_file_ext(p), "HDM") == 0 ||\
			strcmp(get_file_ext(p), "dup") == 0 ||\
			strcmp(get_file_ext(p), "DUP") == 0 ||\
			strcmp(get_file_ext(p), "2HD") == 0 ||\
			strcmp(get_file_ext(p), "2hd") == 0 ||\
			strcmp(get_file_ext(p), "tfd") == 0 ||\
			strcmp(get_file_ext(p), "TFD") == 0 ||\
			strcmp(get_file_ext(p), "d88") == 0 ||\
			strcmp(get_file_ext(p), "D88") == 0 )
	        {
			printf("POS:%d (%s)\n",pos-2,p);
			loaddisk[pos-2]=pos-2;//load_floppy=p-1; break;
		}
		else {
			printf("error command: %s\n", p);
			//goto np2main_err1;
		}
	}

	initload();

	rand_setseed((SINT32)time(NULL));

	TRACEINIT();

	if (fontmng_init() != SUCCESS) {
		goto np2main_err2;
	}
	mousemng_initialize();
	sdlkbd_initialize();
	inputmng_init();
	keystat_initialize();

	if (sysmenu_create() != SUCCESS) {
		goto np2main_err3;
	}

	scrnmng_initialize();
	if (scrnmng_create(FULLSCREEN_WIDTH, FULLSCREEN_HEIGHT) != SUCCESS) {
		goto np2main_err4;
	}

	soundmng_initialize();
	commng_initialize();
	sysmng_initialize();
	taskmng_initialize();
	pccore_init();
	S98_init();

	scrndraw_redraw();
	pccore_reset();

	for(pos=0;pos<argc;pos++)
		if(loaddisk[pos]!=-1 && loaddisk[pos]<4){
			printf("load disk (%s) in drive %d \n",argv[pos+1],pos);
			diskdrv_setfdd(pos,argv[pos+1], 0/*read_only*/);
		}
		else if(loaddisk[pos]!=-1 && loaddisk[pos]>3){
			printf("load harddrive (%s) in drive %d \n",argv[pos+1],pos);
			diskdrv_setsxsi(pos /*drive_num*/, argv[pos+1]);
		}

#if defined(SUPPORT_RESUME)
	if (np2oscfg.resume) {
		id = flagload(str_sav, str_resume, FALSE);
		if (id == DID_CANCEL) {
			goto np2main_err5;
		}
	}
#endif

	return(SUCCESS);

#if 0
	while(taskmng_isavail()) {
		taskmng_rol();
		if (np2oscfg.NOWAIT) {
			pccore_exec(framecnt == 0);
			if (np2oscfg.DRAW_SKIP) {			// nowait frame skip
				framecnt++;
				if (framecnt >= np2oscfg.DRAW_SKIP) {
					processwait(0);
				}
			}
			else {							// nowait auto skip
				framecnt = 1;
				if (timing_getcount()) {
					processwait(0);
				}
			}
		}
		else if (np2oscfg.DRAW_SKIP) {		// frame skip
			if (framecnt < np2oscfg.DRAW_SKIP) {
				pccore_exec(framecnt == 0);
				framecnt++;
			}
			else {
				processwait(np2oscfg.DRAW_SKIP);
			}
		}
		else {								// auto skip
			if (!waitcnt) {
				UINT cnt;
				pccore_exec(framecnt == 0);
				framecnt++;
				cnt = timing_getcount();
				if (framecnt > cnt) {
					waitcnt = framecnt;
					if (framemax > 1) {
						framemax--;
					}
				}
				else if (framecnt >= framemax) {
					if (framemax < 12) {
						framemax++;
					}
					if (cnt >= 12) {
						timing_reset();
					}
					else {
						timing_setcount(cnt - framecnt);
					}
					framereset(0);
				}
			}
			else {
				processwait(waitcnt);
				waitcnt = framecnt;
			}
		}
	}

	pccore_cfgupdate();
	if (np2oscfg.resume) {
		flagsave(str_sav);
	}
	else {
		flagdelete(str_sav);
	}
	pccore_term();
	S98_trash();
	soundmng_deinitialize();

	sysmng_deinitialize();

	scrnmng_destroy();
	sysmenu_destroy();
	TRACETERM();
	SDL_Quit();
	return(SUCCESS);
#endif
#if defined(SUPPORT_RESUME)
np2main_err5:
	pccore_term();
	S98_trash();
	soundmng_deinitialize();
#endif	/* defined(SUPPORT_RESUME) */

np2main_err4:
	scrnmng_destroy();

np2main_err3:
	sysmenu_destroy();

np2main_err2:
	TRACETERM();
	//SDL_Quit();

np2main_err1:
	return(FAILURE);
}

int np2_loop(){

	//while(taskmng_isavail())
	{
		taskmng_rol();
		if (np2oscfg.NOWAIT) {
			pccore_exec(framecnt == 0);
			if (np2oscfg.DRAW_SKIP) {			// nowait frame skip
				framecnt++;
				if (framecnt >= np2oscfg.DRAW_SKIP) {
					processwait(0);
				}
			}
			else {							// nowait auto skip
				framecnt = 1;
				if (timing_getcount()) {
					processwait(0);
				}
			}
		}
		else if (np2oscfg.DRAW_SKIP) {		// frame skip
			if (framecnt < np2oscfg.DRAW_SKIP) {
				pccore_exec(framecnt == 0);
				framecnt++;
			}
			else {
				processwait(np2oscfg.DRAW_SKIP);
			}
		}
		else {								// auto skip
			if (!waitcnt) {
				UINT cnt;
				pccore_exec(framecnt == 0);
				framecnt++;
				cnt = timing_getcount();
				if (framecnt > cnt) {
					waitcnt = framecnt;
					if (framemax > 1) {
						framemax--;
					}
				}
				else if (framecnt >= framemax) {
					if (framemax < 12) {
						framemax++;
					}
					if (cnt >= 12) {
						timing_reset();
					}
					else {
						timing_setcount(cnt - framecnt);
					}
					framereset(0);
				}
			}
			else {
				processwait(waitcnt);
				waitcnt = framecnt;
			}
		}
	}

}

int np2_end(){

	pccore_cfgupdate();
#if defined(SUPPORT_RESUME)
	if (np2oscfg.resume) {
		flagsave(str_sav);
	}
	else {
		flagdelete(str_sav);
	}
#endif
	pccore_term();
	S98_trash();
	soundmng_deinitialize();

	sysmng_deinitialize();

	scrnmng_destroy();
	sysmenu_destroy();
	TRACETERM();
	//SDL_Quit();
	return(SUCCESS);

}

