#include "../../core/bagasmi.h"

const char Platform[] = "NDS";


void Timer_start(u32 *increment){
  *increment = 0;
  cpuStartTiming(0);
}

float Timer_getTicks(u32 *increment){
  unsigned int finalTime = cpuEndTiming();
  return (float)(timerTicks2usec(finalTime)/1000000.0f);
}

int getTime(char *array){
  time_t unixTime = time(NULL);
  struct tm* timeStruct = gmtime((const time_t *)&unixTime);

  int i = 0;
  array[i] = timeStruct->tm_sec;i++;
  array[i] = timeStruct->tm_min;i++;
  array[i] = timeStruct->tm_hour;i++;
  array[i] = -1;i++;
  array[i] = timeStruct->tm_mday;i++;
  array[i] = timeStruct->tm_mon;i++;
  array[i] = timeStruct->tm_year;

  return 1;
}

void waitExit(void){
  iprintf("Press any key to continue...\n");
  int pressed = 0;
  while(!pressed){
    scanKeys();
    pressed = keysDown();
    swiWaitForVBlank();
  }
  exit(0);
}

