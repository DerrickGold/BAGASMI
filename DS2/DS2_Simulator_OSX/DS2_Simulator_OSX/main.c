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

#include "../../../core/bagasmi.h"
#include "../../../core/scriptstack.h"

extern unsigned time(time_t *);
const char Platform[] = "DS2 Simulator";


extern void waitExit(void);

void waitExit(void){
    while(Pad.key){
        BAG_Update();
    }
    while(!Pad.key){
        BAG_Update();
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
  struct rtc time;
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
int main(int argc, char *argv[]){
    if(!BAG_Init(1))
        ds2_plug_exit();
  //BAG_Core_SetFPS(120);
  BAG_DBG_Init(NULL, DBG_LIB);
  
  
    time_t t;
    srand((unsigned)time(&t));
  //memset(&Main, 0, sizeof(ASMSys));

  if(argc >= 1){
    //initialize program arguments from terminal
    char *newArgv[argc];
    for(int i = 0; i < argc-2; i++){
      newArgv[i] = argv[i + 2];
    }

      
#if defined(USE_SCRIPT_STACK)
      System_ScriptsInit();
      if(System_PushScript(argv[1], NULL, 0, newArgv, argc - 2)) {
          while(System_ScriptsRunning()) System_ProcessScripts(0);
      } else {
          printf("failed to push script!\n");
      }
#else
    ASMSys Main = {};
    ASM_InitARGV(&Main, argc - 2, newArgv);
    ASM_run(&Main, argv[1]);
#endif
      
  }
  else
   printf("no file specified\n");
  waitExit();
  return 0;
}

