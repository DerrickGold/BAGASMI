/*---------------------------------------------------------------------------------
 BAGASM  Copyright (C) 2011 - 2012
  BassAceGold - <BassAceGold@gmail.com>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.

  3. Restricting the usage of or portions of code in this library
     from other software developers and or content creators
     is strictly prohibited in applications with software creation capabilities
     and or with the intent to profit from said restrictions.

  4. This notice may not be removed or altered from any source
     distribution.
---------------------------------------------------------------------------------*/

#include "../../core/video.h"
#include <stdarg.h>

//initialize console on bottom screen
void ds_initConsole(void){
    consoleDemoInit();
}


#if defined(BASM_COMPILE_VIDEO)


#if defined(COLLISION_DETECTION)
int cpu_checkCollision(ASMSys *system, int sprite1Num, int sprite2Num){

   /* SDL_Surface *spr1 = system->video.sprite[sprite1Num],
                *spr2 = system->video.sprite[sprite2Num];

    register int w1 = spr1->w,
                 h1 = spr1->h,
                 x1 = system->video.spX[sprite1Num] + (w1 >> 1),
                 y1 = system->video.spY[sprite1Num] + (h1 >> 1),

                 w2 = spr2->w,
                 h2 = spr2->h,
                 x2 = system->video.spX[sprite2Num] + (w2 >> 1),

    //rectangular collision logic
    if((x2 >= x1 - ((w1 + w2)>>1)) && (x2 <= x1 + ((w1 + w2)>>1)) && (y2 >= y1 - ((h1 + h2)>>1)) && (y2 <= y1 + ((h1 + h2)>>1))){
        SET_FLAG(system->cpu.status, BIT_COLLISION);
        return 1;
    }   */
    return 0;
}
#endif

/*===================================================================================
*   DS video output
*
*
*
===================================================================================*/

int ASM_videoInit(ASMSys *system){
  if(system->settings.console == 0){
    SetMode(MODE_3 | BG2_ON);

    system->video.screen[0] = (unsigned short*)VRAM;
    system->video.screen[1] = (unsigned short*)VRAM;
    ASM_clearScreen(system, 0, 0);
  }
  return 1;
}

inline void ASM_setPixel(ASMSys *system, int screen, int x, int y, unsigned short col){
	ASMVideo *vid = &system->video;
	if(x >= 0 && y >= 0 && x < system->settings.screenWidth && y < system->settings.screenHeight)
		vid->screen[screen][x + (y*system->settings.screenWidth)] = col;
}

inline void ASM_setPixelFast(ASMSys *system, int screen, int x, int y, int col){
  system->video.screen[screen][x + (y*system->settings.screenWidth)] = col;
}

void ASM_loadSprite(ASMSys *system, const char *file, int sprite_num){
}

static inline void videoDrawSprites(ASMSys *system){
}

inline void ASM_clearScreen(ASMSys *system, int screen, unsigned short col){
  register int size = 0;
  for(size = 0; size < (system->settings.screenWidth * system->settings.screenHeight); size++)
    system->video.screen[screen][size] = col;
}

inline void ASM_flip(ASMSys *system, int screen){
	videoDrawSprites(system);
}

inline void ASM_Vsync(ASMSys *system){
	VBlankIntrWait();
}

void ASM_freeVideo(ASMSys *system){

}

#endif//BASM_COMPILE_VIDEO
