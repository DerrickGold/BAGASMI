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


const char Platform[] = "DS2";


extern void waitExit(void);

void waitExit(void){
    BAG_Update();
    Pad.key = 0;
    mdelay(12);
    while(Pad.key == 0 || (Pad.key & KEY_R)){
        BAG_Update();
        if(Pad.key & KEY_R){
          BAG_Display_ScrnCap(DUAL_SCREEN, "/");
        }
    }
    ds2_plug_exit();
}

void Timer_start(u32 *increment){
  *increment = getSysTime();
}

float Timer_getTicks(u32 *increment){
  u32 finalTime = getSysTime() - *increment;
  return (finalTime * 49.667)/1000000;
}


int getTime(char *array){
  struct rtc time ={};
  ds2_getTime(&time);
  int i = 0;
  array[i] = time.seconds;i++;
  array[i] = time.minutes;i++;
  array[i] = time.hours;i++;
  array[i] = time.weekday;i++;
  array[i] = time.day;i++;
  array[i] = time.month;i++;
  array[i] = time.year;
  return 1;
}


//get launched path
Argv_t argv;

void ds2_main(void){
	if(!BAG_Init(1))
		ds2_plug_exit();

  BAG_Core_SetFPS(120);
  BAG_DBG_Init(NULL, DBG_HALT | DBG_LIB);

  BAG_Filesystem_GetArgs(&argv);
  ASMSys Main;
  memset(&Main, 0, sizeof(ASMSys));

  ASM_run(&Main, argv.input[1]);
  printf("\nPress any key to exit.");
  BAG_Filesystem_FreeArgs(&argv);
  waitExit();
}

