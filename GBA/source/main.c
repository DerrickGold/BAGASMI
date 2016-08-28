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


#include "../../core/bagasmi.h"
#include "hello_asm.h"
#include "iterations_asm.h"
#include "benchmark_asm.h"
#include "circles_asm.h"
#include "pixelFixer_asm.h"

const char Platform[] = "GBA";


void waitExit(void){
  /*int pressed = 0;
  while(!pressed){
    scanKeys();
    pressed = keysDown();
    swiWaitForVBlank();
  }
  exit(0);*/
  while(1){

  }
}

void Vblank() {
}


void Timer_start(u32 *increment){
  *increment = 0;
  REG_TM2CNT_L= 0;
  REG_TM3CNT_L= 0;

  REG_TM2CNT_H = 0;  REG_TM3CNT= 0;
  REG_TM3CNT_H = TIMER_START | TIMER_COUNT;
  REG_TM2CNT_H = TIMER_START;
}

float Timer_getTicks(u32 *increment){
  REG_TM2CNT_H = 0;
  *increment = (REG_TM3CNT_L<<16)|REG_TM2CNT_L;
  return (float)(*increment * 16.78)/(1000000000.0f);
}


//set console for top screen
//PrintConsole topScreen;
extern void ds_initConsole(void);

static ASMSys Main;
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
  irqInit();
  irqEnable(IRQ_VBLANK);
  ds_initConsole();

  memset(&Main, 0, sizeof(ASMSys));

  //Main.bin = (u8*)&hello_asm;
  //Main.binSize = hello_asm_size;

  Main.bin = (u8*)&iterations_asm;
  Main.binSize = iterations_asm_size;

  //Main.bin = (u8*)&benchmark_asm;
  //Main.binSize = benchmark_asm_size;

  //Main.bin = (u8*)&circles_asm;
  //Main.binSize = circles_asm_size;

  //Main.bin = (u8*)&pixelFixer_asm;
  //Main.binSize = pixelFixer_asm_size;

  ASM_run(&Main, NULL);
  waitExit();
  return 0;
}
