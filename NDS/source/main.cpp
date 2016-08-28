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
#include "file_browse.h"
#include "shared.h"

extern "C" void ds_initConsole(void);


//set console for top screen

//extern void ds_initConsole(void);

//hacked in splash screen using nflib
void splashScreen(void){
  //set bottom screen up for 16 bit bg
  NF_Set2D(1, 5);
  NF_SetRootFolder("NITROFS");
  NF_InitBitmapBgSys(1, 1);
  NF_Init16bitsBgBuffers();
  NF_Load16bitsBg("splash", 0);
  NF_Copy16bitsBuffer(1, 0, 0);

  //int pressed = 0;
  //while(!pressed){
  //  scanKeys();
  //  pressed = keysDown();
  //  swiWaitForVBlank();
  //}
  NF_Unload16bitsBg(0);
  //NF_SetRootFolder("FAT");
}

void Vblank() {
}

extern "C" void textEditor(const char *filePath);

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
  irqSet(IRQ_VBLANK, Vblank);

  ds_initConsole();

  if(!nitroFSInit()){
    printf("failed to initiate fat!\n");
    while(1){
      //idle here
    }
  }

  //textEditor(NULL);


  ASMSys Main = {};

  while(true){

    splashScreen();

    //browser
    char filePath[MAX_PATH];
    std::string filename;
    filename = browseForFile (".asm");

    getcwd (filePath, MAXPATHLEN);
    int pathLen = strlen (filePath);
    strcpy (filePath + pathLen, filename.c_str());

    //make sure no keys are pressed
    while(keysCurrent()){
      scanKeys();
      swiWaitForVBlank();
    }

    //run file
    ASM_run(&Main, filePath);
    waitExit();
  }
  return 0;
}
