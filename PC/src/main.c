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
const char Platform[] = "PC-Windows";

void waitExit(void){
    SDL_Event keyevent;
    while(keyevent.type != SDL_KEYDOWN){
        SDL_PollEvent(&keyevent);
    }
    SDL_Quit();
    exit(0);
}

int main(int argc, char *argv[]){
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        printf("error initializing SDL\n");

  freopen( "CON", "w", stdout );
  freopen( "CON", "w", stderr );
  ASMSys Main = {};

  //printf("\nBAGASM PC Interpreter\n\n");

  if(argc >= 1)
    ASM_run(&Main, argv[1]);
  else
   printf("no file specified\n");

  printf("\nPress any key to exit.");
  waitExit();
  return EXIT_SUCCESS;
}


