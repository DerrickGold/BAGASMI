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

//libgba
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <gba_base.h>
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <gba_input.h>



//#define BASM_HAS_FILESYSTEM
#define BASM_DEFAULT_CONSOLE 0

#define BASM_COMPILE_VIDEO
#define BASM_COMPILE_INPUT
#define BASM_ALLOW_LOOPS


#define VIDEO_STRUCT\
  unsigned short *screen[2];


#define RUNTIME
#define CPU_LITTLE_ENDIAN

//use the libnds console
#define printf iprintf

//marks what platform the .basm file was created for
#define COMPILED_FOR (ASM_PLATFORM_GBA)

//some defines used from DSTwo library
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160
#define RGB15(r,g,b)  ((r)+(g<<5)+(b<<10)) 

#define SET_FLAG(Var, Flag) (Var|=Flag)
#define RESET_FLAG(Var, Flag) (Var &= ~Flag)
#define CLEAR_FLAGS(Var,Val) (Var=Val)
#define GET_FLAG(Var, Flag) ((Var & Flag)!=0)
#define MAX_PATH 128

#define BASM_LABELLEN 26
#define BASM_STRLEN 128
#define BASM_ARGLEN 32

/*
constants of the same value are shared instead
of having their each individual instance in ram.

May be slower compiling on larger BAGASM programs.
*/
#define OPTIMIZE_CONSTANT_STORAGE

#define DBGTEXT
//#define OUTPUT_LINES_FILE

//store virtual ram on the stack
//#define BASM_USE_STACK_MEM
#define BASM_DIR_SEPARATOR '/'
#ifdef __cplusplus
}
#endif

#endif
