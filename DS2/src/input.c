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

#include "../../core/input.h"

#if defined(BASM_COMPILE_INPUT)

bagAddrPtr *ASM_setStylusAddr(ASMSys *system, char *label){
  bagAddrPtr *data = NULL;

  if(!strncmp(label, STYLUS_X, strlen(STYLUS_X)))//stylus x position
    data = (bagAddrPtr*)system->input.x;

  else if(!strncmp(label, STYLUS_Y, strlen(STYLUS_Y)))//stylus y position
    data = (bagAddrPtr*)system->input.y;

  return data;
}


/*===================================================================================
*   DSTwo input detection
*
*
*
===================================================================================*/
//from libBAG
extern unsigned short _BAG_ExPad;
extern void _equalityJump(ASMSys *system);

static void ASM_getKey(ASMSys *system){
  //if running as script, user can't force control updates
  if(!system->settings.passive)
      BAG_UpdateIN();

  (*system->input.pad) = Pad.key;

  if(*system->input.pad & KEY_TOUCH){
  	system->cpu.reg[STYX_REG] = Stylus.X;
  	system->cpu.reg[STYY_REG] = Stylus.Y;
      //printf("Stylys: %d,%d\n", system->cpu.reg[STYX_REG], system->cpu.reg[STYY_REG]);
  }

  return;
}

static void chkkey(ASMSys *system){
  system->cpu.status = 0;
  system->cpu.curVal = GET_FLAG((*system->cpu.arg[0]), (*system->cpu.arg[1]));

  //check if theres a jump statement
  if(system->cpu.curVal && system->cpu.arg[2]){
    _equalityJump(system);
  }
  return;
}

static void chkkeypress(ASMSys *system){
  system->cpu.status = 0;
  system->cpu.curVal = ((*system->cpu.arg[0]) & (~_BAG_ExPad)) & (*system->cpu.arg[1]);
  //check if theres a jump statement
  if(system->cpu.curVal && system->cpu.arg[2]){
    _equalityJump(system);
  }
  return;
}

static void chkkeyrelease(ASMSys *system){
  system->cpu.status = 0;
  system->cpu.curVal = (_BAG_ExPad & (~(*system->cpu.arg[0]))) & (*system->cpu.arg[1]);
  //check if theres a jump statement
  if(system->cpu.curVal && system->cpu.arg[2]){
    _equalityJump(system);
  }
  return;
}

static void getkey(ASMSys *system){
    system->cpu.status = 0;
    ASM_getKey(system);
    system->cpu.curVal = (*system->cpu.arg[0]) = (*system->input.pad);
    return;
}


static void asm_initInputOps(ASMSys *system){
  system->global->inputOpInit++;
  if(system->global->inputOpInit > 1) return;

  ASM_setCpuOpsEx(system->global, "GETKEY", 1, &getkey);
  ASM_setCpuOpsEx(system->global, "CHKKEY", 2, &chkkey);
  ASM_setCpuOpsEx(system->global, "CHKKEYNEW", 2, &chkkeypress);
  ASM_setCpuOpsEx(system->global, "CHKKEYOLD", 2, &chkkeyrelease);
  return;
}

void ASM_initInput(ASMSys *system){
  asm_initInputOps(system);

  system->input.x = (bagAddrPtr*)&system->cpu.reg[STYX_REG];
  system->input.y = (bagAddrPtr*)&system->cpu.reg[STYY_REG];
  system->cpu.reg[PAD_REG] = 0;
  system->input.pad = (bagAddrPtr*)&system->cpu.reg[PAD_REG];
  return;
  //&Pad.key;
}


#endif//BASM_COMPILE_INPUT
