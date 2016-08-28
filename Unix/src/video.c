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

#include "../../core/video.h"

#if defined(BASM_COMPILE_VIDEO)

const char Window_Title[] = "BAGASMI Video Output";


#if defined(COLLISION_DETECTION)
int cpu_checkCollision(ASMSys *system, int sprite1Num, int sprite2Num){

    SDL_Surface *spr1 = system->video.sprite[sprite1Num],
                *spr2 = system->video.sprite[sprite2Num];

    register int w1 = spr1->w,
                 h1 = spr1->h,
                 x1 = system->video.spX[sprite1Num] + (w1 >> 1),
                 y1 = system->video.spY[sprite1Num] + (h1 >> 1),

                 w2 = spr2->w,
                 h2 = spr2->h,
                 x2 = system->video.spX[sprite2Num] + (w2 >> 1),
                 y2 = system->video.spY[sprite2Num] + (h2 >> 1);

    //rectangular collision logic
    if((x2 >= x1 - ((w1 + w2)>>1)) && (x2 <= x1 + ((w1 + w2)>>1)) && (y2 >= y1 - ((h1 + h2)>>1)) && (y2 <= y1 + ((h1 + h2)>>1))){
        SET_FLAG(system->cpu.status, BIT_COLLISION);
        return 1;
    }
    return 0;
}
#endif

/*===================================================================================
*   PC video output
*
*
*
===================================================================================*/
//extern int Timer_getTicks(struct Timer_s *timer);

struct Timer_s Timer;
void Timer_Start(struct Timer_s *timer){
    //Start the timer
    timer->started = 1;

    //Unpause the timer
    timer->paused = 0;

    //Get the current clock time
    timer->startTicks = SDL_GetTicks();
}

unsigned int Timer_getTicks(struct Timer_s *timer){
    //If the timer is running
    if( timer->started == 1 ){
        //If the timer is paused
        if( timer->paused == 1 ){
            //Return the number of ticks when the timer was paused
            return timer->pausedTicks;
        }
        else{
            //Return the current time minus the start time
            return SDL_GetTicks() - timer->startTicks;
        }
    }

    //If the timer isn't running
    return 0;
}


static void SDL_putPixel(SDL_Surface *obj, int x, int y, unsigned short color){
	if(x >= 0 && y >= 0 && x < obj->w && y < obj->h){
		unsigned short *bufp = (unsigned short *)obj->pixels + y*obj->pitch/2 + x;
		*bufp = color;
	}
}


static unsigned short *SDL_getPixel(SDL_Surface *obj, int x, int y){
	unsigned short *bufp = (unsigned short *)obj->pixels + y*obj->pitch/2 + x;
	return bufp;
}

static void SDL_drawSprite(SDL_Surface *sprite, SDL_Surface *scrn, int x, int y){
	SDL_Rect dest;
    dest.x = x;
    dest.y = y;
   SDL_BlitSurface(sprite, NULL, scrn, &dest);
}

static void SDL_vsyc(void){
	if(Timer_getTicks(&Timer) < 1000 / 60 ){
		//Sleep the remaining frame time
		SDL_Delay( ( 1000 / 60 ) - Timer_getTicks(&Timer));
	}
    //#if defined(PLATFORM_PC)
        Timer_Start(&Timer);
    //#endif
}

static void fillScreen(SDL_Surface *scrn, unsigned short col){
  SDL_Rect r;
  r.x = 0;
  r.y = 0;
  r.w = scrn->w;
  r.h = scrn->h;
  SDL_FillRect(scrn, &r, col);
}

/*======================================================================================
Initialization
======================================================================================*/
static void freeScreen(ASMSys *system, int screen){
  if(system->video.screen[screen])
    SDL_FreeSurface(system->video.screen[screen]);
  system->video.screen[screen] = NULL;
}

static int initScreen(ASMSys *system, int screen){
  freeScreen(system, screen);
  system->video.screen[screen] = SDL_CreateRGBSurface(SDL_SWSURFACE, system->settings.screenWidth, system->settings.screenHeight, 16, 0, 0, 0, 0);
  if(system->video.screen[screen] == NULL)
    return 0;
  return 1;
}

static void freeSprites(ASMSys *system){
  for(int i = 0; i < BASM_MAX_SPRITES; i++){
    if(system->video.sprite[i]!= NULL)
      SDL_FreeSurface(system->video.sprite[i]);
    
    system->video.sprite[i] = NULL;
  }
}

static int initSprites(ASMSys *system){
  freeSprites(system);
  for(int i = 0; i < BASM_MAX_SPRITES; i++){
    system->video.sprite[i] = NULL;
    system->video.updateSprites[i] = -1;
  }
  return 1;
}



int ASM_screenInit(ASMSys *system){
    if(system->settings.isPlugin){
        int v = system->settings.screen;
        initScreen(system, v);
        //map other screen to the one screen
        system->video.screen[(v+1)%2] = system->video.screen[v]; 

    }
    else{//normal screen initialization
        for(int i = 0; i < 2; i++){
            if(!initScreen(system, i))
                return 0;
        }
    }
    return 1;
}



int ASM_videoInit(ASMSys *system){
	printf("initiating video\n");
	//make window
	system->video.window = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT * 2, 16, SDL_SWSURFACE);

  if ( system->video.window == NULL ){
  	errorMsg(system, "Error Initializing Window", 0, 0);
    return 0;
  }

	SDL_WM_SetCaption(Window_Title, NULL );
  if(!ASM_screenInit(system))
    return 0;

  if(!initSprites(system))
    return 0;

	return 1;
}

void ASM_freeVideo(ASMSys *system){
  if(system->settings.isPlugin)//only clear the one screen allocated
    freeScreen(system, system->settings.screen);
  else{
    freeScreen(system, 0);
    freeScreen(system, 1);
  }
  freeSprites(system);
}
/*======================================================================================
BAGASMI API
======================================================================================*/

static void ASM_setPixel(ASMSys *system, int screen, int x, int y, unsigned short col){
	ASMVideo *vid = &system->video;
	SDL_putPixel(vid->screen[screen], x, y, col);
}

//#define ASM_setPixelFast ASM_setPixel
inline void ASM_setPixelFast(ASMSys *system, int screen, int x, int y, int col){
    ASM_setPixel(system, screen, x, y, col);
}

static void ASM_loadSprite(ASMSys *system, const char *file, int sprite_num){
	char newLine[256];
    strcpy(newLine, system->curRoot);
    newLine[strlen(newLine) - 1] = BASM_DIR_SEPARATOR;
    strcat(newLine, file);

	system->video.tempSpr = SDL_LoadBMP(newLine);
	if ( system->video.tempSpr  == NULL ){
		printf("file: %s\n", file);
		errorMsg(system, "No sprite found", system->prgm.pos, 0);
	}
	else{
		 system->video.sprite[sprite_num] = SDL_DisplayFormat(system->video.tempSpr);
		 SDL_FreeSurface(system->video.tempSpr);
	}
}

static void videoDrawSprites(ASMSys *system){
	ASMVideo *vid = &system->video;
	register int i = 0;
	do{
		if(vid->updateSprites[i] > -1){
			SDL_drawSprite(vid->sprite[i], vid->screen[vid->updateSprites[i]], vid->spX[i], vid->spY[i]);
			vid->updateSprites[i] = -1;
		}
	}while(++i < BASM_MAX_SPRITES);
}

static void ASM_clearScreen(ASMSys *system, int screen, unsigned short col){
	fillScreen(system->video.screen[screen], col);
}

static void ASM_flip(ASMSys *system, int screen){
	videoDrawSprites(system);
	SDL_Rect dest;
	dest.x = system->settings.x;
	dest.y = system->settings.y;

  if(system->settings.isPlugin && screen != system->settings.screen)
    screen = system->settings.screen;

	if(screen == 0)
		dest.y += SCREEN_HEIGHT;

	SDL_BlitSurface(system->video.screen[screen], NULL, system->video.window, &dest);
	SDL_Flip(system->video.window);
}

static void ASM_Vsync(ASMSys *system){
	SDL_vsyc();
}


#if defined(BASM_COMPILE_ADVANCED_VIDEO)
  int ASM_ResizeVideo(ASMSys *system, int newWd, int newHt){
      //ASM_freeVideo(system);
      ASM_clearScreen(system, 0, 0);
      ASM_clearScreen(system, 1, 0);
      system->settings.screenWidth = newWd;
      system->settings.screenHeight = newHt;
      return ASM_screenInit(system);
  }
#endif

#endif//BASM_COMPILE_VIDEO
