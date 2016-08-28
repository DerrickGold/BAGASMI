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


#ifndef _ASM_CONFIG_
#define _ASM_CONFIG_

#ifdef __cplusplus
extern "C" {
#endif

#include <libBAG.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

//#define USE_64BIT 1
//#define BASM_NO_FORCED_EXIT
#define BASM_HAS_FILESYSTEM
#define BASM_COMPILE_VIDEO
#define BASM_COMPILE_ADVANCED_VIDEO
#define BASM_COMPILE_FAT
#define BASM_COMPILE_INPUT
#define BASM_COMPILE_EXTLIBS
#define BASM_ALLOW_LOOPS
#define BASM_ALLOW_SPRITES
#define BASM_ALLOW_BACKGROUNDS
#define BASM_ALLOW_STRINGS
#define BASM_ALLOW_FONTS
#define ENABlE_TIME
#define BASM_NO_ALIGN_MEM

#define BASM_MAX_SPRITES 16
#define BASM_MAX_BACKGROUNDS 4
#define BASM_MAX_FONTS 4
#define BASM_MAX_TEXTBOXS 4


#define VIDEO_STRUCT \
  void **outScreen[2];\
  GFXObj_t *screen[2],\
           *sprite[BASM_MAX_SPRITES];\
  TiledBG_t *background[BASM_MAX_BACKGROUNDS]; \
            int bgX[BASM_MAX_BACKGROUNDS], \
                bgY[BASM_MAX_BACKGROUNDS]; \
  FNTObj_t  *font[BASM_MAX_FONTS]; \
  TextBox_t fontBox[BASM_MAX_TEXTBOXS]; \
  void *gfxQue;

#define EXTLIBS_DATA \
  INIObj_t iniFile;



#define RUNTIME
#define CPU_LITTLE_ENDIAN

#define COLLISION_DETECTION
//compile for ds2
#define COMPILED_FOR (ASM_PLATFORM_DS2)

/*
constants of the same value are shared instead
of having their each individual instance in ram.

May be slower compiling on larger BAGASM programs.
*/
#define OPTIMIZE_CONSTANT_STORAGE

#define DBGTEXT
#define COUNT_OPERATIONS
#define OUTPUT_LINES_FILE
#define DUMP_OPS_LIST

//store virtual ram on the stack
#define BASM_USE_STACK_MEM
#define BASM_DIR_SEPARATOR '/'

//#define BASM_MEMSIZE (1024*6)
#ifdef __cplusplus
}
#endif

#endif
