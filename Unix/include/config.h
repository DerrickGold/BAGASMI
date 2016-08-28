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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/usr/include/SDL/SDL.h"

#define BASM_HAS_FILESYSTEM
#define BASM_NO_FORCED_EXIT

#define USE_TIMER_SYS
#define BASM_COMPILE_VIDEO
#define BASM_COMPILE_ADVANCED_VIDEO
#define BASM_COMPILE_FAT
#define BASM_COMPILE_INPUT
#define BASM_ALLOW_LOOPS
#define BASM_ALLOW_SPRITES
#define BASM_ALLOW_STRINGS

#define VIDEO_STRUCT \
  SDL_Surface *window,\
              *screen[2], \
              *sprite[BASM_MAX_SPRITES], \
              *tempSpr; \
  int spX[BASM_MAX_SPRITES], \
      spY[BASM_MAX_SPRITES];



#define COUNT_OPERATIONS
#define CPU_LITTLE_ENDIAN
#define COLLISION_DETECTION

#define COMPILED_FOR (ASM_PLATFORM_PC)

//some defines used from DSTwo library
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 192
#define MAX_PATH 512

#define BIT(a) (1<<a)
#define RGB15(r,g,b)  (((b)|((g)<<6)|((r)<<11))-32768)

#define SET_FLAG(Var, Flag) (Var|=Flag)
#define RESET_FLAG(Var, Flag) (Var &= ~Flag)
#define CLEAR_FLAGS(Var,Val) (Var=Val)
#define GET_FLAG(Var, Flag) ((Var & Flag)!=0)

/*
constants of the same value are shared instead
of having their each individual instance in ram.

May be slower compiling on larger BAGASM programs.
*/
#define OPTIMIZE_CONSTANT_STORAGE

#define DBGTEXT
#define OUTPUT_LINES_FILE

//store virtual ram on the stack
#define BASM_USE_STACK_MEM
#define BASM_DIR_SEPARATOR '/'

#define BASM_ADDRESS_TYPE intptr_t

struct Timer_s{
    int startTicks;
    int pausedTicks;
    char paused;
    char started;
};
extern struct Timer_s Timer;

#ifdef __cplusplus
}
#endif

#endif
