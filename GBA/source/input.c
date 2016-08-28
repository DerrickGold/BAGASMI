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

#include "input.h"

#if defined(BASM_COMPILE_INPUT)
/*===================================================================================
*   DS input detection
*
*
*
===================================================================================*/
void ASM_initInput(ASMSys *system){
	system->input.x = (int*)&system->cpu.reg[STYX_REG];
	system->input.y = (int*)&system->cpu.reg[STYY_REG];
	system->cpu.reg[PAD_REG] = 0;
	system->input.pad = (int*)&system->cpu.reg[PAD_REG];
}

inline void ASM_getKey(ASMSys *system){
	//update pad
	scanKeys();
	*system->input.pad = keysDown();
  //keysHeld();

  /*if(*system->input.pad & KEY_TOUCH){
  	//update touch pad
  	touchPosition touch;
  	touchRead(&touch);
  	*system->input.x = touch.px;
  	*system->input.y = touch.py;
  }*/
}
#endif//BASM_COMPILE_INPUT
