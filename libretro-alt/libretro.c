
#include <stdio.h>
#include <stdlib.h>  
#include <ctype.h>
#include <string.h>

#include "libretro.h"

#include "compiler.h"
#include "strres.h"
#include "np2.h"
#include "dosio.h"
#include "fontmng.h"
#include "scrnmng.h"
#include "soundmng.h"
#include "sysmng.h"
#include "taskmng.h"
#include "joymng.h"
#include "mousemng.h"
#include "ini.h"
#include "pccore.h"
#include "iocore.h"
#include "scrndraw.h"
#include "fddfile.h"
#include "newdisk.h"
#include "diskdrv.h"
#include "timing.h"
#include "keystat.h"
#include "vramhdl.h"
#include "menubase.h"
#include "sysmenu.h"

#ifdef _WIN32
char slash = '\\';
#else
char slash = '/';
#endif

#define SOUNDRATE 44100.0
#define SNDSZ 735

char RPATH[512];
char RETRO_DIR[512];
const char *retro_save_directory;
const char *retro_system_directory;
const char *retro_content_directory;
char retro_system_conf[512];

char Core_Key_Sate[512];
char Core_old_Key_Sate[512];

bool opt_analog;

int retrow=640;
int retroh=400;
int CHANGEAV=0;

int pauseg=0;

signed short soundbuf[1024*2];

uint16_t videoBuffer[640*400];  //emu  surf
uint16_t videoBuffer2[640*400]; //menu surf

static retro_video_refresh_t video_cb;
static retro_environment_t environ_cb;

static  retro_input_poll_t input_poll_cb;

retro_input_state_t input_state_cb;
retro_audio_sample_t audio_cb;
retro_audio_sample_batch_t audio_batch_cb;

void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb  =cb; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

#include <sys/time.h>

long GetTicks(void)
{ // in MSec
#ifndef _ANDROID_

#ifdef __CELLOS_LV2__

   //#warning "GetTick PS3\n"

   unsigned long	ticks_micro;
   uint64_t secs;
   uint64_t nsecs;

   sys_time_get_current_time(&secs, &nsecs);
   ticks_micro =  secs * 1000000UL + (nsecs / 1000);

   return ticks_micro/1000;
#else
   struct timeval tv;
   gettimeofday (&tv, NULL);
   return (tv.tv_sec*1000000 + tv.tv_usec)/1000;
#endif

#else

   struct timespec now;
   clock_gettime(CLOCK_MONOTONIC, &now);
   return (now.tv_sec*1000000 + now.tv_nsec/1000)/1000;
#endif

} 

static char CMDFILE[512];

int loadcmdfile(char *argv)
{
   int res=0;

   FILE *fp = fopen(argv,"r");

   if( fp != NULL )
   {
      if ( fgets (CMDFILE , 512 , fp) != NULL )
         res=1;	
      fclose (fp);
   }

   return res;
}

int HandleExtension(char *path,char *ext)
{
   int len = strlen(path);

   if (len >= 4 &&
         path[len-4] == '.' &&
         path[len-3] == ext[0] &&
         path[len-2] == ext[1] &&
         path[len-1] == ext[2])
   {
      return 1;
   }

   return 0;
}
//Args for experimental_cmdline
static char ARGUV[64][1024];
static unsigned char ARGUC=0;

// Args for Core
static char XARGV[64][1024];
static const char* xargv_cmd[64];
int PARAMCOUNT=0;

extern int cmain(int argc, char *argv[]);

void parse_cmdline( const char *argv );

void Add_Option(const char* option)
{
   static int first=0;

   if(first==0)
   {
      PARAMCOUNT=0;	
      first++;
   }

   sprintf(XARGV[PARAMCOUNT++],"%s\0",option);
}

int pre_main(const char *argv)
{
   int i=0;
   int Only1Arg;

   if (strlen(argv) > strlen("cmd"))
   {
      if( HandleExtension((char*)argv,"cmd") || HandleExtension((char*)argv,"CMD"))
         i=loadcmdfile((char*)argv);     
   }

   if(i==1)
      parse_cmdline(CMDFILE);      
   else
      parse_cmdline(argv); 

   Only1Arg = (strcmp(ARGUV[0],"np21") == 0) ? 0 : 1;

   for (i = 0; i<64; i++)
      xargv_cmd[i] = NULL;


   if(Only1Arg)
   {  
      int cfgload=0;

      Add_Option("np21");
/*
      if (strlen(RPATH) >= strlen("hdf")){
         if(!strcasecmp(&RPATH[strlen(RPATH)-strlen("hdf")], "hdf")){
            Add_Option("-h");
            cfgload=1;
         }
      }
*/
      if(cfgload==0)
      {
         //Add_Option("-verbose");
         //Add_Option(retro_system_tos);
         //Add_Option("-8");
      }

      Add_Option(RPATH);
   }
   else
   { // Pass all cmdline args
      for(i = 0; i < ARGUC; i++)
         Add_Option(ARGUV[i]);
   }

   for (i = 0; i < PARAMCOUNT; i++)
   {
      xargv_cmd[i] = (char*)(XARGV[i]);
      printf("arg_%d:%s\n",i,xargv_cmd[i]);
   }

   char tmppath[4096]; 

   sprintf(tmppath,"%s%c",retro_system_conf,slash);

   dosio_init();
   file_setcd(tmppath);

   np2_main(PARAMCOUNT,( char **)xargv_cmd); 

   xargv_cmd[PARAMCOUNT - 2] = NULL;

   return 0;
}

void parse_cmdline(const char *argv)
{
   char *p,*p2,*start_of_word;
   int c,c2;
   static char buffer[512*4];
   enum states { DULL, IN_WORD, IN_STRING } state = DULL;

   strcpy(buffer,argv);
   strcat(buffer," \0");

   for (p = buffer; *p != '\0'; p++)
   {
      c = (unsigned char) *p; /* convert to unsigned char for is* functions */
      switch (state)
      {
         case DULL: /* not in a word, not in a double quoted string */
            if (isspace(c)) /* still not in a word, so ignore this char */
               continue;
            /* not a space -- if it's a double quote we go to IN_STRING, else to IN_WORD */
            if (c == '"')
            {
               state = IN_STRING;
               start_of_word = p + 1; /* word starts at *next* char, not this one */
               continue;
            }
            state = IN_WORD;
            start_of_word = p; /* word starts here */
            continue;
         case IN_STRING:
            /* we're in a double quoted string, so keep going until we hit a close " */
            if (c == '"')
            {
               /* word goes from start_of_word to p-1 */
               //... do something with the word ...
               for (c2 = 0,p2 = start_of_word; p2 < p; p2++, c2++)
                  ARGUV[ARGUC][c2] = (unsigned char) *p2;
               ARGUC++; 

               state = DULL; /* back to "not in word, not in string" state */
            }
            continue; /* either still IN_STRING or we handled the end above */
         case IN_WORD:
            /* we're in a word, so keep going until we get to a space */
            if (isspace(c))
            {
               /* word goes from start_of_word to p-1 */
               //... do something with the word ...
               for (c2 = 0,p2 = start_of_word; p2 <p; p2++,c2++)
                  ARGUV[ARGUC][c2] = (unsigned char) *p2;
               ARGUC++; 

               state = DULL; /* back to "not in word, not in string" state */
            }
            continue; /* either still IN_WORD or we handled the end above */
      }	
   }
}

void texture_init(void)
{
   memset(videoBuffer, 0,retrow*retroh*2);
} 

void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;

   struct retro_variable variables[] = {
      {
         "np2_analog","Use Analog; OFF|ON",
      },
      { NULL, NULL },
   };

   cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);

}

static void update_variables(void)
{
   struct retro_variable var = {0};


   var.key = "np2_analog";
   var.value = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      fprintf(stderr, "value: %s\n", var.value);
      if (!strcmp(var.value, "OFF"))
         opt_analog = false;
      if (!strcmp(var.value, "ON"))
         opt_analog = true;

      fprintf(stderr, "[libretro-test]: Analog: %s.\n",opt_analog?"ON":"OFF");
   }
}

static const char *cross[] = {
  "X                               ",
  "XX                              ",
  "X.X                             ",
  "X..X                            ",
  "X...X                           ",
  "X....X                          ",
  "X.....X                         ",
  "X......X                        ",
  "X.......X                       ",
  "X........X                      ",
  "X.....XXXXX                     ",
  "X..X..X                         ",
  "X.X X..X                        ",
  "XX  X..X                        ",
  "X    X..X                       ",
  "     X..X                       ",
  "      X..X                      ",
  "      X..X                      ",
  "       XX                       ",
  "                                ",
};

void DrawPointBmp(unsigned short *buffer,int x, int y, unsigned short color)
{
   int idx;

   idx=x+y*retrow;
   buffer[idx]=color;	
}


void draw_cross(int x,int y) {

	int i,j,idx;
	int dx=32,dy=20;
	unsigned  short color;

	for(j=y;j<y+dy;j++){
		idx=0;
		for(i=x;i<x+dx;i++){
			if(cross[j-y][idx]=='.')DrawPointBmp(videoBuffer,i,j,0xffff);
			else if(cross[j-y][idx]=='X')DrawPointBmp(videoBuffer,i,j,0);
			idx++;			
		}
	}

}

#define KEYP(a,b) {\
	if(Core_Key_Sate[a] && Core_Key_Sate[a]!=Core_old_Key_Sate[a]  )\
		sdlkbd_keydown(a);\
	else if ( !Core_Key_Sate[a] && Core_Key_Sate[a]!=Core_old_Key_Sate[a]  )\
		sdlkbd_keyup(a);\
}	

static int lastx=320,lasty=200;

void update_input(void)
{

  		input_poll_cb();

  		joymng_sync();

		int i;
		static int mposx=320,mposy=200;

   		for(i=0;i<320;i++)
      			Core_Key_Sate[i]=input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0,i) ? 0x80: 0;

   		if(memcmp( Core_Key_Sate,Core_old_Key_Sate , sizeof(Core_Key_Sate) ) )
			for(i=0;i<320;i++){
				KEYP(i,i);
			}

		if(Core_Key_Sate[RETROK_F11] && Core_Key_Sate[RETROK_F11]!=Core_old_Key_Sate[RETROK_F11]  )
			if (menuvram == NULL) {
				sysmenu_menuopen(0, 0, 0);
				mposx=0;mposy=0;
				lastx=0;lasty=0;
				mousemng_disable(MOUSEPROC_SYSTEM);
			}
			else {
				menubase_close();
				mousemng_enable(MOUSEPROC_SYSTEM);
				memset(videoBuffer2,0,retrow*retroh*2);
			}

   		memcpy(Core_old_Key_Sate,Core_Key_Sate , sizeof(Core_Key_Sate) );

		static int mbL = 0, mbR = 0;

	      	int mouse_x = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
		int mouse_y = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);

		if (!mousemng.flag){			
			mousemng_sync(mouse_x,mouse_y);
		}

		mposx+=mouse_x;if(mposx<0)mposx=0;if(mposx>=retrow)mposx=retrow-1;
		mposy+=mouse_y;if(mposy<0)mposy=0;if(mposy>=retroh)mposy=retroh-1;

		if(lastx!=mposx || lasty!=mposy)
			if (menuvram == NULL) {
			}
			else {
				menubase_moving(mposx, mposy, 0);
			}

		int mouse_l    = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT);
		int mouse_r    = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT);
		      
  	        if(mbL==0 && mouse_l){
      			mbL=1;		

			if (menuvram != NULL)
			{
				menubase_moving(mposx, mposy, 1);
			}
			else mousemng_buttonevent(MOUSEMNG_LEFTDOWN);
			
		}
   		else if(mbL==1 && !mouse_l)
   		{
   			mbL=0;
			if (!mousemng_buttonevent(MOUSEMNG_LEFTUP))
			{
				if (menuvram != NULL)
				{
					menubase_moving(mposx, mposy, 2);
				}
				else
				{
					sysmenu_menuopen(0, mposx, mposy);
				}
			}

		}
  	        if(mbR==0 && mouse_r){
      			mbR=1;		
			mousemng_buttonevent(MOUSEMNG_RIGHTDOWN);
		}
   		else if(mbR==1 && !mouse_r)
   		{
   			mbR=0;
			mousemng_buttonevent(MOUSEMNG_RIGHTUP);
		}

		lastx=mposx;lasty=mposy;

}


#if 0
static void keyboard_cb(bool down, unsigned keycode, uint32_t character, uint16_t mod)
{
}
#endif

/************************************
 * libretro implementation
 ************************************/

//static struct retro_system_av_info g_av_info;

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name = "np21";
   info->library_version = "0.86";
   info->need_fullpath = true;
   info->valid_extensions = "d98|zip|98d|fdi|fdd|2hd|tfd|d88|88d|hdm|xdf|dup|cmd|hdi|thd|nhd|hdd";
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   /* FIXME handle PAL/NTSC */
   struct retro_game_geometry geom = { retrow, retroh,640, 400 ,4.0 / 3.0 };
   struct retro_system_timing timing = { 60.0, SOUNDRATE };

   info->geometry = geom;
   info->timing   = timing;
}

void update_geometry(void)
{
   struct retro_system_av_info system_av_info;
   system_av_info.geometry.base_width = retrow;
   system_av_info.geometry.base_height = retroh;
   system_av_info.geometry.aspect_ratio = (float)4/3;// retro_aspect;
   environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &system_av_info);
}


void retro_set_controller_port_device(unsigned port, unsigned device)
{
    (void)port;
    (void)device;
}

size_t retro_serialize_size(void)
{
	return 0;
}

bool retro_serialize(void *data, size_t size)
{
    return false;
}

bool retro_unserialize(const void *data, size_t size)
{
    return false;
}

void retro_cheat_reset(void)
{}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
    (void)index;
    (void)enabled;
    (void)code;
}

bool retro_load_game(const struct retro_game_info *info)
{
   const char *full_path;

   full_path = info->path;

   strcpy(RPATH,full_path);

   printf("LOAD EMU\n");

   return true;
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
    (void)game_type;
    (void)info;
    (void)num_info;
    return false;
}

void retro_unload_game(void)
{
     pauseg=0;
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void *retro_get_memory_data(unsigned id)
{
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    return 0;
}

void retro_init(void)
{
   const char *system_dir = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &system_dir) && system_dir)
   {
      // if defined, use the system directory			
      retro_system_directory=system_dir;		
   }		   

   const char *content_dir = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY, &content_dir) && content_dir)
   {
      // if defined, use the system directory			
      retro_content_directory=content_dir;		
   }			

   const char *save_dir = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &save_dir) && save_dir)
   {
      // If save directory is defined use it, otherwise use system directory
      retro_save_directory = *save_dir ? save_dir : retro_system_directory;      
   }
   else
   {
      // make retro_save_directory the same in case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY is not implemented by the frontend
      retro_save_directory=retro_system_directory;
   }

   if(retro_system_directory==NULL)sprintf(RETRO_DIR, "%s\0",".");
   else sprintf(RETRO_DIR, "%s\0", retro_system_directory);

   sprintf(retro_system_conf, "%s%cnp2\0",RETRO_DIR,slash);

   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;

   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      fprintf(stderr, "RGB565 is not supported.\n");
      exit(0);
   }

   struct retro_input_descriptor inputDescriptors[] = {
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "X" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Y" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2, "R2" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2, "L2" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3, "R3" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3, "L3" }
	};
	environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, &inputDescriptors);

/*
    struct retro_keyboard_callback cbk = { keyboard_cb };
    environ_cb(RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK, &cbk);
*/
  	update_variables();

    memset(Core_Key_Sate,0,512);
    memset(Core_old_Key_Sate ,0, sizeof(Core_old_Key_Sate));
}

void retro_deinit(void)
{
   np2_end();
   printf("Retro DeInit\n");
}

void retro_reset(void)
{
}

static int firstcall=1;

void retro_run(void)
{       
   if(firstcall)
   {
      pre_main(RPATH);
      mousemng_enable(MOUSEPROC_SYSTEM);
      firstcall=0;
      printf("INIT done\n");
      return;
   }

   if (CHANGEAV == 1)
   {
      update_geometry();
      printf("w:%d h:%d a:%f\n",retrow,retroh,(float)(4/3));
      CHANGEAV=0;
   }

   update_input();

   if (menuvram != NULL){
	memcpy(videoBuffer,videoBuffer2,retrow*retroh*2);
	draw_cross(lastx,lasty);
   }
   else {

 	   pccore_exec(TRUE);
	   sound_play_cb(NULL, NULL,SNDSZ*4);
   }


   video_cb(videoBuffer, retrow, retroh, /*retrow*/ 640 << 1/*2*/);
}

