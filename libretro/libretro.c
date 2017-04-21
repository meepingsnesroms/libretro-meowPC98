#include <stdio.h>
#include <stdint.h>
#ifndef _MSC_VER
#include <stdbool.h>
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "libretro.h"
#include "libretro_params.h"

#include "compiler.h"//required to prevent missing type errors
#include "pccore.h"
#include "keystat.h"
#include "fddfile.h"
#include "newdisk.h"
#include "diskdrv.h"
#include "soundmng.h"
#include "sysmng.h"
#include "font.h"
#include "keyboardtranslate.h"

static retro_log_printf_t log_cb = NULL;
static retro_video_refresh_t video_cb = NULL;
static retro_input_poll_t poll_cb = NULL;
static retro_input_state_t input_cb = NULL;
static retro_environment_t environ_cb = NULL;

uint16_t   FrameBuffer[LR_SCREENWIDTH * LR_SCREENHEIGHT];
retro_audio_sample_batch_t audio_batch_cb = NULL;

char* get_file_ext(char* filepath){
   return filepath + strlen(filepath) - 3;
}

void updateInput(){
   
   poll_cb();
   
   uint32_t i;
   for (i=0; i < keys_needed; i++)
      if (input_cb(0, RETRO_DEVICE_KEYBOARD, 0, keys_to_poll[i])){
         send_libretro_key_down(keys_to_poll[i]);
      }
      else {
         send_libretro_key_up(keys_to_poll[i]);
      }
}

//dummy functions
void *retro_get_memory_data(unsigned type){return NULL;}
size_t retro_get_memory_size(unsigned type){return 0;}
void retro_set_audio_sample(retro_audio_sample_t cb){}
bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info){return false;}
void retro_unload_game (void){}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
   input_cb = cb;
}

void retro_set_environment(retro_environment_t cb)
{
   struct retro_log_callback logging;
   
   environ_cb = cb;
   
   //bool no_rom = !LR_REQUIRESROM;
   //environ_cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_rom);
   
   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
      log_cb = logging.log;
   else
      log_cb = NULL;
   

}

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->need_fullpath    = LR_NEEDFILEPATH;
   info->valid_extensions = LR_VALIDFILEEXT;
   info->library_version  = LR_LIBVERSION;
   info->library_name     = LR_CORENAME;
   info->block_extract    = LR_BLOCKARCEXTRACT;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   info->geometry.base_width   = LR_SCREENWIDTH;
   info->geometry.base_height  = LR_SCREENHEIGHT;
   info->geometry.max_width    = LR_SCREENWIDTH;
   info->geometry.max_height   = LR_SCREENHEIGHT;
   info->geometry.aspect_ratio = LR_SCREENASPECT;
   info->timing.fps            = LR_SCREENFPS;
   info->timing.sample_rate    = LR_SOUNDRATE;
}

void retro_init (void)
{
   enum retro_pixel_format rgb565;
   

   rgb565 = RETRO_PIXEL_FORMAT_RGB565;
   if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb565) && log_cb)
         log_cb(RETRO_LOG_INFO, "Frontend supports RGB565 - will use that instead of XRGB1555.\n");
   
   init_lr_key_to_pc98();
}

void retro_deinit(void)
{
   pccore_cfgupdate();
   pccore_term();
   S98_trash();
   soundmng_deinitialize();
   
   sysmng_deinitialize();
   
   scrnmng_destroy();
   sysmenu_destroy();
   TRACETERM();
}

void retro_reset (void)
{
   pccore_reset();
}

void retro_run (void)
{
   updateInput();
   
   //emulate 1 frame
   pccore_exec(true /*draw*/);
   
   video_cb(FrameBuffer, LR_SCREENWIDTH, LR_SCREENHEIGHT, LR_SCREENWIDTH * 2/*Pitch*/);
}

size_t retro_serialize_size (void)
{
   //no savestates on this core
   return 0;
}

bool retro_serialize(void *data, size_t size)
{
   //no savestates on this core
   return false;
}

bool retro_unserialize(const void * data, size_t size)
{
   //no savestates on this core
   return false;
}

void retro_cheat_reset(void)
{
   //no cheats on this core
} 

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   //no cheats on this core
}

bool retro_load_game(const struct retro_game_info *game)
{
   
   //get system dir
   const char* syspath = 0;
   char np2path[4096];
   bool worked = environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &syspath);
   if(!worked)abort();
   
   strcpy(np2path, syspath);
#ifdef _WIN32
   strcat(np2path, "\\np2\\");
#else 
   strcat(np2path, "/np2/");
#endif

   file_setcd(np2path);//set current directory
   
   initload();
   
   //retroarch will handle the audio latency
   np2cfg.delayms = 0;
   
   strcpy(np2cfg.fontfile, np2path);
   strcat(np2cfg.fontfile, "font.bmp");
   
   strcpy(np2cfg.biospath, np2path);
   strcat(np2cfg.biospath, "BIOS.ROM");

#define FILETYPE(x) if(strcmp(get_file_ext(game->path), x) == 0)
   if(game){
      printf("PATH:%s\n", game->path);
      printf("EXT:%s\n", get_file_ext(game->path));
      FILETYPE("hdi"){
         //SASI hdd
         diskdrv_setsxsi(0 /*drive_num*/, game->path);
      }
      FILETYPE("vhd"){
         //SASI hdd
         diskdrv_setsxsi(0 /*drive_num*/, game->path);
      }
      FILETYPE("thd"){
         //SASI hdd
         diskdrv_setsxsi(0 /*drive_num*/, game->path);
      }
      FILETYPE("nhd"){
         //SASI hdd
         diskdrv_setsxsi(0 /*drive_num*/, game->path);
      }
      FILETYPE("fdd"){
         //floppy disk
         diskdrv_setfdd(0, game->path, 0/*read_only*/);
      }
   }
#undef FILETYPE(x)
   
   TRACEINIT();
   if(fontmng_init() != SUCCESS){
      printf("Font init failed.\n");
      abort();//hack
   }
   inputmng_init();
   keystat_initialize();
   if(sysmenu_create() != SUCCESS){
      printf("Sysmenu init failed.\n");
      abort();//hack
   }
   scrnmng_initialize();
   if(scrnmng_create(LR_SCREENWIDTH, LR_SCREENHEIGHT) != SUCCESS){
      printf("Scrnmng init failed.\n");
      abort();//hack
   }
   soundmng_initialize();
   commng_initialize();
   sysmng_initialize();
   taskmng_initialize();
   font_initialize();
   pccore_init();
   S98_init();
   
   scrndraw_redraw();
   pccore_reset();
   
   return true;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   (void)port;
   (void)device;
}

unsigned retro_get_region (void)
{ 
   return RETRO_REGION_NTSC;
}
