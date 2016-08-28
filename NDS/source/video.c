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

extern void NDS_InitConsoleTop(void);
//initialize console on bottom screen
void ds_initConsole(void){
  NDS_InitConsoleTop();
}


#if defined(BASM_COMPILE_VIDEO)

static short dest_screen[2];


//initialize the 16 bit bg
void ds_init16BitBg(int screen){
    switch(screen){
        case 1://top screen
            videoSetMode(MODE_5_2D);
            vramSetBankA(VRAM_A_MAIN_BG);
            dest_screen[1] = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0,0);
        break;
        case 0://bottom screen
            videoSetModeSub(MODE_5_2D);
            vramSetBankC(VRAM_C_SUB_BG);
            dest_screen[0] = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0,0);
        break;
    }
}

//set the addresses for video hardware
BASM_ADDRESS_TYPE *ASM_setVideoAddr(ASMSys *system, char *label){
  BASM_ADDRESS_TYPE *data = NULL;

  //background scroll offsets
  #if defined(BASM_ALLOW_BACKGROUNDS)
      if(!strncmp(label, BACKGROUND_Y, strlen(BACKGROUND_Y))){//sprite y position
          label += strlen(BACKGROUND_Y);
          data = (BASM_ADDRESS_TYPE*)&system->video.bgY[BASM_atoi(label)];
          //return setAddr((BASM_ADDRESS_TYPE*)&system->video.bgY[BASM_atoi(label)], output);
      }
      else if(!strncmp(label, BACKGROUND_X, strlen(BACKGROUND_X))){//sprite y position
          label += strlen(BACKGROUND_X);
          data = (BASM_ADDRESS_TYPE*)&system->video.bgX[BASM_atoi(label)];
      }
  #endif
  return data;
}

/*===================================================================================
*   DS video output
*
*
*
===================================================================================*/

static void _setPixel(ASMSys *system){
  //grab function args
  int screen = (*system->cpu.arg[0]),
      x = (*system->cpu.arg[1]),
      y = (*system->cpu.arg[2]);
  unsigned short col = (unsigned short)(*system->cpu.arg[3]);

	if(x >= 0 && y >= 0 && x < system->settings.screenWidth && y < system->settings.screenHeight)
		system->video.screen[screen][x + (y*system->settings.screenWidth)] = col;
}

static void _setPixelRGB(ASMSys *system){
  //build color from given arguments
  char red = (*system->cpu.arg[3]),
       green = (*system->cpu.arg[4]),
       blue = (*system->cpu.arg[5]);

  //pass new color to third argument for set pixel function
  BASM_ADDRESS_TYPE col = RGB15(red, green, blue);
  system->cpu.arg[3] = &col;
  _setPixel(system);
  return;
}

static void _setPixelFast(ASMSys *system){
    int screen = (*system->cpu.arg[0]),
      x = (*system->cpu.arg[1]),
      y = (*system->cpu.arg[2]);
  unsigned short col = (unsigned short)(*system->cpu.arg[3]);
  system->video.screen[screen][x + (y*system->settings.screenWidth)] = col;
}

static void _rgb(ASMSys *system){
    (*system->cpu.arg[0]) = RGB15((unsigned char)(*system->cpu.arg[1]),
                                  (unsigned char)(*system->cpu.arg[2]),
                                  (unsigned char)(*system->cpu.arg[3]));
    return;
}

static void _clearScreen(ASMSys *system){
  int screen = (*system->cpu.arg[0]);
  unsigned short color = 0;
  if(system->cpu.arg[1]) color = (*system->cpu.arg[1]);

	register int size = (SCREEN_WIDTH*SCREEN_HEIGHT)<<1;
	dmaFillHalfWords(color, system->video.screen[screen], size);//256*256*2
}

static void _flip(ASMSys *system){
  int screen = (*system->cpu.arg[0]);

	dmaCopyHalfWordsAsynch (0,
			(unsigned short*)system->video.screen[screen],
			(unsigned short*)bgGetGfxPtr(dest_screen[screen]),
			(SCREEN_WIDTH*SCREEN_HEIGHT)<<1
		);
}

static void _vsync(ASMSys *system){
	swiWaitForVBlank();
}

void ASM_freeVideo(ASMSys *system){
	free(system->video.screen[0]);
  free(system->video.screen[1]);
	/*BAG_Display_DeleteObj(system->video.screen[0]);
	asm_free(system->video.screen[0], sizeof(GFXObj_t));
	BAG_Display_DeleteObj(system->video.screen[1]);
	asm_free(system->video.screen[1], sizeof(GFXObj_t));
	int i = 0;
	for(i = 0; i < BASM_MAX_SPRITES; i++){
		BAG_Display_DeleteObj(system->video.sprite[0]);
		asm_free(system->video.sprite[0], sizeof(GFXObj_t));
	}
	memset(&system->video,0, sizeof(ASMVideo));
	ds2_clearScreen(DOWN_SCREEN, RGB15(0,0,0));*/
}


static void ASM_initVidOps(ASMSys *system){
  system->global->vidOpInit++;
  if(system->global->vidOpInit > 1) return;

  #if defined(COLLISION_DETECTION)
      ASM_setCpuOpsEx(system->global, "CHKCOL", 2, &chkcol);
  #endif

  ASM_setCpuOpsEx(system->global, "FLIP", 1, &_flip);
  ASM_setCpuOpsEx(system->global, "VSYNC", 0, &_vsync);
  ASM_setCpuOpsEx(system->global, "SETPIXEL", 4, &_setPixel);
  ASM_setCpuOpsEx(system->global, "SETPIXELF", 4, &_setPixelFast);
  ASM_setCpuOpsEx(system->global, "SETPIXRGB", 6, &_setPixelRGB);
  ASM_setCpuOpsEx(system->global, "CLRSCRN", 1, &_clearScreen);
  ASM_setCpuOpsEx(system->global, "RGB", 4, &_rgb);
  ASM_setCpuOpsEx(system->global, "SETSCRNCOL", 2, &_clearScreen);
  //ASM_setCpuOpsEx(system->global, "DRWLINE", 7, &_drawLine);
  //ASM_setCpuOpsEx(system->global, "DRWRECT", 6, &_drawRec);

  #if defined(BASM_COMPILE_ADVANCED_VIDEO)
  //  ASM_setCpuOpsEx(system->global, "SETVIDSIZE", 2, &ASM_ResizeVideo);
  #endif

  #if defined(BASM_ALLOW_SPRITES)
      ASM_setCpuOpsEx(system->global, "DRWSPR", 2, &_drawSpr);
      ASM_setCpuOpsEx(system->global, "LDSPR", 2, &_loadSpr);
      ASM_setCpuOpsEx(system->global, "DELSPR", 1, &_deleteSpr);
      ASM_setCpuOpsEx(system->global, "SPRFRAMEDIM", 3, &_setSprFrameDim);
      ASM_setCpuOpsEx(system->global, "SPRFRAME", 3, &_setSprFrame);
      ASM_setCpuOpsEx(system->global, "CLONESPR", 2, &_cloneSpr);
      ASM_setCpuOpsEx(system->global, "SETSPRXY", 3, &_setSprXY);
      ASM_setCpuOpsEx(system->global, "SETSPRALPHA", 2, &_setSprAlpha);
  #endif

  #if defined(BASM_ALLOW_BACKGROUNDS)
      ASM_setCpuOpsEx(system->global, "DELBG", 1, &_deleteBG);
      ASM_setCpuOpsEx(system->global, "LDBG", 2, &_loadBG);
      ASM_setCpuOpsEx(system->global, "DRWBG", 2, &_drawBG);
  #endif

  #if defined(BASM_ALLOW_FONTS)
      ASM_setCpuOpsEx(system->global, "LDFNT", 2, &_loadFont);
      ASM_setCpuOpsEx(system->global, "FNTPRINT", 4, &_fontPrint);
      ASM_setCpuOpsEx(system->global, "FNTSETCOL", 2, &_fontCol);
      ASM_setCpuOpsEx(system->global, "TBOXINIT", 8, &_textBoxProperties);

  #endif

  return;
}

int ASM_videoInit(ASMSys *system){
  if(!system->settings.screenWidth)
    system->settings.screenWidth = SCREEN_WIDTH;

  if(!system->settings.screenHeight)
    system->settings.screenHeight = SCREEN_HEIGHT;

  ASM_initVidOps(system);

    for (int screen = 0; screen < (2 - system->settings.console); screen++){
      ds_init16BitBg(screen);
      system->video.screen[screen] = calloc(system->settings.screenWidth * system->settings.screenHeight, sizeof(unsigned short));
      if(!system->video.screen[screen])
        errorMsg(system, -1, "Error allocating screens");

    }
  /*
  for(i = 0; i < BASM_MAX_SPRITES; i++){
    system->video.sprite[i] = asm_calloc(1, sizeof(GFXObj_t));
    if(system->video.sprite[i] == NULL)
      errorMsg(system, "Error allocating sprites", i, 0);

    system->video.updateSprites[i] = -1;
  }*/
  return 1;
}

#endif//BASM_COMPILE_VIDEO
