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

void updateInput(){
   //void keystat_keydown(REG8 ref);
   //void keystat_keyup(REG8 ref);
}

static retro_log_printf_t log_cb = NULL;
static retro_video_refresh_t video_cb = NULL;
static retro_input_poll_t poll_cb = NULL;
static retro_input_state_t input_cb = NULL;
static retro_audio_sample_t audio_cb = NULL;
static retro_audio_sample_batch_t audio_batch_cb = NULL;
static retro_environment_t environ_cb = NULL;


uint16_t FrameBuffer[LR_SCREENWIDTH * LR_SCREENHEIGHT];


void *retro_get_memory_data(unsigned type)
{
   return NULL;
}

size_t retro_get_memory_size(unsigned type)
{
   return 0;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
   audio_cb = cb;
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
   
   initload();
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
   pccore_init();
   S98_init();
}

void retro_deinit(void)
{
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
   if (!game)
      return false;
   
   char* file_extension = game->path + strlen(game->path) - 3;
   
   if(strcmp(file_extension, "fdd") == 0){
      //floppy disk, all oters are harddrives
      newdisk_fdd(game->path, DISKTYPE_2HD/*type, largest floppy*/, "NoName");
   }
   else if(strcmp(file_extension, "thd") == 0){
      diskdrv_setsxsi(0/*drive number*/, game->path);
   }
   else if(strcmp(file_extension, "nhd") == 0){
      diskdrv_setsxsi(0/*drive number*/, game->path);
   }
   else if(strcmp(file_extension, "hdi") == 0){
      diskdrv_setsxsi(0/*drive number*/, game->path);
   }
   else if(strcmp(file_extension, "vhd") == 0){
      diskdrv_setsxsi(0/*drive number*/, game->path);
   }
   else return false;
   
   char* syspath;
   uint32_t syspathlength;
   bool worked = environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY,(void *)&syspath);
   if(!worked)abort();
   syspathlength = strlen(syspath);
   
   strcpy(np2cfg.fontfile, syspath);
   strcpy(syspath + syspathlength, "/NP2/FONT.BMP");
   
   strcpy(np2cfg.biospath, syspath);
   strcpy(syspath + syspathlength, "/NP2/BIOS.ROM");
   
   np2cfg.EXTMEM = 7;//max memory a stock pc98 can have
   
   pccore_reset();
   
   return true;
}

bool retro_load_game_special(
  unsigned game_type,
  const struct retro_game_info *info, size_t num_info
)
{
   return false;
}

void retro_unload_game (void)
{

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
