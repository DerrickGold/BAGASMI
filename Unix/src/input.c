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
/*===================================================================================
*   PC input detection
*
*
*	might be a better way to do this...
===================================================================================*/

void ASM_initInput(ASMSys *system){
	system->input.x = (int*)&system->cpu.reg[STYX_REG];
	system->input.y = (int*)&system->cpu.reg[STYY_REG];
	system->cpu.reg[PAD_REG] = 0;
	system->input.pad = (int*)&system->cpu.reg[PAD_REG];
}

void ASM_getKey(ASMSys *system){
	SDL_Event keyevent;
	while (SDL_PollEvent(&keyevent)){//Poll our SDL key event for any keystrokes.
		switch(keyevent.type){
			case SDL_KEYDOWN:
				switch(keyevent.key.keysym.sym){
					case SDLK_LEFT:
					  	SET_FLAG((*system->input.pad), BIT(5));
					break;
					case SDLK_RIGHT:
					  	SET_FLAG((*system->input.pad), BIT(4));
					break;
					case SDLK_UP:
					  	SET_FLAG((*system->input.pad), BIT(6));
					break;
					case SDLK_DOWN:
				  		SET_FLAG((*system->input.pad), BIT(7));
					break;
					case SDLK_z://mapped to A button
						SET_FLAG((*system->input.pad), BIT(0));
					break;
					case SDLK_x://mapped to B button
						SET_FLAG((*system->input.pad), BIT(1));
					break;
					case SDLK_a://mapped to X button
						SET_FLAG((*system->input.pad), BIT(10));
					break;
					case SDLK_s://mapped to Y button
						SET_FLAG((*system->input.pad), BIT(11));
					break;
					case SDLK_RETURN://mapped to start button
						SET_FLAG((*system->input.pad), BIT(3));
					break;
					case SDLK_SPACE://mapped to select button
						SET_FLAG((*system->input.pad), BIT(2));
					break;
					case SDLK_q://mapped to L button
						SET_FLAG((*system->input.pad), BIT(9));
					break;
					case SDLK_w://mapped to R button
						SET_FLAG((*system->input.pad), BIT(8));
					break;
					default:
					break;
				}
			break;
			case SDL_KEYUP://reset keys on release
				switch(keyevent.key.keysym.sym){
					case SDLK_LEFT:
					  	RESET_FLAG((*system->input.pad), BIT(5));
					break;
					case SDLK_RIGHT:
					  	RESET_FLAG((*system->input.pad), BIT(4));
					break;
					case SDLK_UP:
					  	RESET_FLAG((*system->input.pad), BIT(6));
					break;
					case SDLK_DOWN:
				  		RESET_FLAG((*system->input.pad), BIT(7));
					break;
					case SDLK_z://mapped to A button
						RESET_FLAG((*system->input.pad), BIT(0));
					break;
					case SDLK_x://mapped to B button
						RESET_FLAG((*system->input.pad), BIT(1));
					break;
					case SDLK_a://mapped to X button
						RESET_FLAG((*system->input.pad), BIT(10));
					break;
					case SDLK_s://mapped to Y button
						RESET_FLAG((*system->input.pad), BIT(11));
					break;
					case SDLK_RETURN://mapped to start button
						RESET_FLAG((*system->input.pad), BIT(3));
					break;
					case SDLK_SPACE://mapped to select button
						RESET_FLAG((*system->input.pad), BIT(2));
					break;
					case SDLK_q://mapped to L button
						RESET_FLAG((*system->input.pad), BIT(9));
					break;
					case SDLK_w://mapped to R button
						RESET_FLAG((*system->input.pad), BIT(8));
					break;
				}
			break;
		}
	}

	//update some mouses stuff
	int mouseX = 0, mouseY = 0;
	int mButtons = SDL_GetMouseState(&mouseX, &mouseY);
	//check if mouse is touching the touch screen (bottom screen)
	RESET_FLAG((*system->input.pad), BIT(12));
	if(mButtons & SDL_BUTTON_LMASK){
		if(mouseY < SCREEN_HEIGHT)//check if mouse is on bottom screen
			return;

		SET_FLAG((*system->input.pad), BIT(12));
		//update mouse location only when screen is touched
		*system->input.x = mouseX;
		*system->input.y = mouseY - SCREEN_HEIGHT;
	}
}
#endif//BASM_COMPILE_INPUT
