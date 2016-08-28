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

//global addresses of the ds2 io layer pointers,
static void **screen_addr[2] = {(void*)&down_screen_addr, (void*)&up_screen_addr};

/*
Manage the order in which objects are drawn
based on their call order in script
*/
//draw up to 32 objects at once
#ifndef BASM_MAX_VIDEOQU
    #define BASM_MAX_VIDEOQU 32
#endif

//the node structure
typedef enum{
    QUEUE_TYPE_SPR = 1,
    QUEUE_TYPE_BG,
    QUEUE_TYPE_FNT,
}VIDEO_QUEUE_TYPES;

//type of data for fonts
typedef struct fontData{
  wString string;//the string data
  TextBox_t box;//output data
}fontData;

//this needs to be 4 bytes alligned!
typedef struct vidQNode{
    int updateSlot, type;
    int screen, destWd, destHt;//destWd
    void *dest;
    
    
    void *data;
}vidQNode __attribute__ ((aligned (4)));

//queing structure
typedef struct videoQueue{
  int curQue;
  vidQNode *nodes[BASM_MAX_VIDEOQU];
}videoQueue;



static void videoQueue_Add(videoQueue *q, const vidQNode *info, void *data){
  if(q->curQue < 0) q->curQue = 0;

  if(q->curQue >= BASM_MAX_VIDEOQU || !info) return;

  //allocate queue slot and copy data
  q->nodes[q->curQue] = calloc(1, sizeof(vidQNode));
  if(!q->nodes[q->curQue]){
    BASM_DBGPRINT("rawr!\n");
    return;
  }
  memcpy(q->nodes[q->curQue], info, sizeof(vidQNode));
  q->nodes[q->curQue]->data = NULL;

  //allocate space to store a copy of the data to draw
  //store sprite data
  if(data){
    switch(info->type){
      //fonts use default case for now possible ToDo
      default:
        q->nodes[q->curQue]->data = data;
      break;

      //Sprite case
      #if defined(BASM_ALLOW_SPRITES)
        case QUEUE_TYPE_SPR:
          q->nodes[q->curQue]->data = calloc(1, sizeof(GFXObj_t));
          if(!q->nodes[q->curQue]->data){
            BASM_DBGPRINT("error allocating queue data\n");
            return;
          } else {
            BAG_Display_CopyObj((GFXObj_t*)q->nodes[q->curQue]->data, (GFXObj_t*)data);
          }
        break;
      #endif

    }//switch
  }//if

  q->curQue++;
  //otherwise, no more slots, cannot queue any more
  return;
}

static vidQNode *videoQueue_Pop(videoQueue *q){
  if(q->curQue > -1){//check that there are things to see
    //always return the first node
    vidQNode *curNode = q->nodes[0];
    //remove one
    q->curQue--;
    if(q->curQue > 0){
      //shift queue now
      for(int i = 0; i < q->curQue; i++) q->nodes[i] =  q->nodes[i + 1];
    }
    return curNode;
  }
  else{//q is empty
    return NULL;
  }
}



//set the addresses for video hardware
bagAddrPtr *ASM_setVideoAddr(ASMSys *system, char *label){
    bagAddrPtr *data = NULL;

  //background scroll offsets
#if defined(BASM_ALLOW_BACKGROUNDS)
    if(!strncmp(label, BACKGROUND_Y, strlen(BACKGROUND_Y))){//sprite y position
        label += strlen(BACKGROUND_Y);
        data = (bagAddrPtr*)&system->video.bgY[BASM_atoi(label)];
          //return setAddr((bagAddrPtr*)&system->video.bgY[BASM_atoi(label)], output);
    }
    else if(!strncmp(label, BACKGROUND_X, strlen(BACKGROUND_X))){//sprite y position
        label += strlen(BACKGROUND_X);
        data = (bagAddrPtr*)&system->video.bgX[BASM_atoi(label)];
    }
    else
#endif
    if (!strncmp(label, LCDSCREEN_BUF, strlen(LCDSCREEN_BUF))) {
        label += strlen(LCDSCREEN_BUF);
        data = (bagAddrPtr*)&screen_addr[BASM_atoi(label)];
    }
    //handle screen buffers
    return data;
}


#if defined(COLLISION_DETECTION)
int cpu_checkCollision(ASMSys *system, int sprite1Num, int sprite2Num){

    GFXObj_t *spr1 = system->video.sprite[sprite1Num],
             *spr2 = system->video.sprite[sprite2Num];

    register int w1 = spr1->data.width,
                 h1 = spr1->data.height,
                 x1 = spr1->blitX + (w1 >> 1),
                 y1 = spr1->blitY + (h1 >> 1),
                 w2 = spr2->data.width,
                 h2 = spr2->data.height,
                 x2 = spr2->blitX + (w2 >> 1),
                 y2 = spr2->blitY + (h2 >> 1);

    //rectangular collision logic
    if((x2 >= x1 - ((w1 + w2)>>1)) && (x2 <= x1 + ((w1 + w2)>>1)) && (y2 >= y1 - ((h1 + h2)>>1)) && (y2 <= y1 + ((h1 + h2)>>1))){
        SET_FLAG(system->cpu.status, BIT_COLLISION);
        return 1;
    }
    return 0;
}
#endif



void asm_fill16bit(unsigned short val, unsigned short *dest, unsigned long size){
    while(--size > 0)
        (*dest++) = val;
    return;
}


inline static int getScreenSize(const ASMSys *system){
  return (system->settings.screenWidth * system->settings.screenHeight) << 1;
}

/*======================================================================================
Initialization
======================================================================================*/


static void freeGfxQue(ASMSys *system){
  if(system->video.gfxQue)
    free(system->video.gfxQue);
  system->video.gfxQue = NULL;
  return;
}

static int initGfxQue(ASMSys *system){
  system->video.gfxQue = calloc(1, sizeof(videoQueue));
  if(!system->video.gfxQue){
    errorMsg(system, -1, "Error allocating graphics queue.\n");
    return 0;
  }
  return 1;
}

static void freeScreen(ASMSys *system, int screen){
    if(system->video.screen[screen]){
        BAG_Display_DeleteObj(system->video.screen[screen]);
        free(system->video.screen[screen]);
    }
    system->video.screen[screen] = NULL;
    return;
}

static int initScreen(ASMSys *system, int screen){
  freeScreen(system, screen);

  system->video.screen[screen] = calloc(1, sizeof(GFXObj_t));
  if(system->video.screen[screen] == NULL){
      errorMsg(system, -1, "Error allocating screens.\n");
      return 0;
  }

  int width = system->settings.screenWidth, height = system->settings.screenHeight;

  int err = BAG_Display_CreateObj(system->video.screen[screen], 16, width, height, width, height);
  if(err != ERR_NONE)
      errorMsg(system, -1, "Error allocating Screen Buffer: Screen:%d, Err:%d\n", screen, err);


  //redefine the default screen output device to this temp one
  system->video.outScreen[screen] = (void *)&system->video.screen[screen]->buffer.gfx;

  //clear display screen
  asm_fill16bit(0, *system->video.outScreen[screen], getScreenSize(system)>>1);

  return 1;
}


#if defined(BASM_ALLOW_SPRITES)
  static void freeSprites(ASMSys *system){
      for(int i = 0; i < BASM_MAX_SPRITES; i++){
          if(system->video.sprite[i]){
              BAG_Display_DeleteObj(system->video.sprite[i]);
              free(system->video.sprite[i]);
          }
          system->video.sprite[i] = NULL;
      }
      return;
  }


  static int initSprites(ASMSys *system){
    freeSprites(system);
    for(int i = 0; i < BASM_MAX_SPRITES; i++){
        system->video.sprite[i] = calloc(1, sizeof(GFXObj_t));
        if(system->video.sprite[i] == NULL){
            errorMsg(system, -1, "Error allocating sprite: %d\n", i);
            return 0;
        }
    }
    return 1;
  }
#endif



#if defined(BASM_ALLOW_BACKGROUNDS)

  static void freeBackgrounds(ASMSys *system){
    for(int i = 0; i < BASM_MAX_BACKGROUNDS; i++){
      if(system->video.background[i]){
        BAG_TileBG_DeleteBG(system->video.background[i]);
        free(system->video.background[i]);
      }
      system->video.background[i] = NULL;
    }
    return;
  }

  static int initBackgrounds(ASMSys *system){
    freeBackgrounds(system);
    for(int i = 0; i < BASM_MAX_BACKGROUNDS; i++){
      system->video.background[i] = calloc(1, sizeof(TiledBG_t));
      if(system->video.background[i] == NULL){
          errorMsg(system, -1, "Error allocating background: %d\n", i);
          return 0;
      }
    }
    return 1;
  }
#endif

#if defined(BASM_ALLOW_FONTS)
  static void freeFonts(ASMSys *system){
    for(int i = 0; i < BASM_MAX_FONTS; i++){
      if(system->video.font[i]){
        BAG_Font_Unload(system->video.font[i]);
        free(system->video.font[i]);
      }
      system->video.font[i] = NULL;
    }
    return;
  }

  static int initFonts(ASMSys *system){
    freeFonts(system);
    for(int i = 0; i < BASM_MAX_FONTS; i++){
      system->video.font[i] = calloc(1, sizeof(FNTObj_t));
      if(!system->video.font[i]){
        errorMsg(system, -1, "Error allocating font: %d\n", i);
        return 0;
      }
    }

    return 1;
  }
#endif


int ASM_screenInit(ASMSys *system){
    for(int i = 0; i < 2; i++){
        if(!initScreen(system, i))
            return 0;
    }
    return 1;
}


void ASM_freeVideo(ASMSys *system){
    freeGfxQue(system);

    freeScreen(system, 0);
    freeScreen(system, 1);

    #if defined(BASM_ALLOW_SPRITES)
      freeSprites(system);
    #endif

    #if defined(BASM_ALLOW_BACKGROUNDS)
      freeBackgrounds(system);
    #endif

    memset(&system->video,0, sizeof(ASMVideo));
    //ds2_clearScreen(DOWN_SCREEN, RGB15(0,0,0));
    return;
}



/*======================================================================================
 BAGASMI NEW VIDEO API
    -Allows direct memory access to graphic objects and the screen :D
 ======================================================================================*/
static void rgb(ASMSys *system){
    (*system->cpu.arg[0]) = RGB15((unsigned char)(*system->cpu.arg[1]),
                                  (unsigned char)(*system->cpu.arg[2]),
                                  (unsigned char)(*system->cpu.arg[3]));
    return;
}


//returns current screen buffer address
static void _getLCDBuffer(ASMSys *system) {
    //register to hold address, screen to get buffer address of
    bagAddrPtr *scrn = NULL;
    if(*system->cpu.arg[1] == 0) scrn = (bagAddrPtr*)*system->video.outScreen[0];
    else scrn = (bagAddrPtr*)*system->video.outScreen[1];
    
    
    (*system->cpu.arg[0]) = (bagAddrPtr)scrn;
}

//essentially a 16 bit memset
static void _gfxMemSet(ASMSys *system) {
    //dest buffer, value to fill, size of dest buffer
    unsigned short *dest = (unsigned short*)*system->cpu.arg[0];
    unsigned short col = *system->cpu.arg[1];
    bagDWord size = (bagDWord)*system->cpu.arg[2];
    while(size-- > 0) *dest++ = col;
    return;
}

static void _gfxSetPix(ASMSys *system){
    //dest buffer, buffer wd, xpos, ypos, color
    unsigned short *dest = (unsigned short *)*system->cpu.arg[0];
    
    
    int wd = (int)*system->cpu.arg[1];
    int x = (int)*system->cpu.arg[2];
    int y = (int)*system->cpu.arg[3];
    
    x = (x < 0) ? 0 : x;
    y = (y < 0) ? 0 : y;
    x = (x >= wd) ? wd - 1 : x;
    
    
    unsigned short col = (unsigned short)*system->cpu.arg[4];
    
    dest[x + (y * wd)] = col;
    return;
}

static void _gfxSetPixRGB(ASMSys *system) {
    int red     = (int)*system->cpu.arg[4],
    green   = (int)*system->cpu.arg[5],
    blue    = (int)*system->cpu.arg[6];
    
    *system->cpu.arg[4] = RGB15(red, green, blue);
    _gfxSetPix(system);
    return;
}

//return pixel color from a specified buffer
static void _gfxGetPix(ASMSys *system){
    //register, dest, destWd, x pos, y pos
    unsigned short *dest = (unsigned short *)*system->cpu.arg[1];
    int    destWd = (int)*system->cpu.arg[2],
                x = (int)*system->cpu.arg[3],
                y = (int)*system->cpu.arg[4];
    
    //printf("x: %d, y: %d, wd: %d\n", x, y, destWd);
    unsigned short col = dest[x + (y * destWd)];
    (*system->cpu.arg[0]) =(bagDWord)col;
    return;
}



static void _gfxDrwLine(ASMSys *system){
    int arg = 0;
    //dest, dest wd, dest ht, x1, y1, x2, y2, color, thickness
    unsigned short *dest = (unsigned short*)*system->cpu.arg[arg++];

    int     destWd = (bagWord)(*system->cpu.arg[arg++]),
            destHt = (bagWord)(*system->cpu.arg[arg++]),
            x1 = (bagWord)(*system->cpu.arg[arg++]),
            y1 = (bagWord)(*system->cpu.arg[arg++]),
            x2 = (bagWord)(*system->cpu.arg[arg++]),
            y2 = (bagWord)(*system->cpu.arg[arg++]);
    
    unsigned short color = (unsigned short)(*system->cpu.arg[7]);
    bagWord size = (bagWord)(*system->cpu.arg[8]);
 
    BAG_Draw_BlitLineEx(dest, destWd, destHt,
                        x1, y1, x2, y2, color, size);
    return;
}

static void _gfxDrwRec(ASMSys *system){
    //dest, dest wd, dest ht, x1, y1, x2, y2, color
    
    int arg = 0;
    unsigned short *dest = (unsigned short*)(*system->cpu.arg[arg++]);
    int     destWd = (bagWord)(*system->cpu.arg[arg++]),
            destHt = (bagWord)(*system->cpu.arg[arg++]),
            x1 = (bagWord)(*system->cpu.arg[arg++]),
            y1 = (bagWord)(*system->cpu.arg[arg++]),
            x2 = (bagWord)(*system->cpu.arg[arg++]),
            y2 = (bagWord)(*system->cpu.arg[arg++]);
    unsigned short color = (unsigned short)(*system->cpu.arg[arg++]);
    
    BAG_Draw_Rect(dest, destWd, destHt,
                  x1, y1,x2, y2, color);
    
    return;
}

/*==============================================
Sprites
==============================================*/
#if defined(BASM_ALLOW_SPRITES)
  static int _checkSprNum(ASMSys *system, int num){
    if(num < 0 || num > BASM_MAX_SPRITES){
      if(GET_FLAG(system->flags, ASM_AUTOERROR)){
        errorMsg(system, system->prgm.pos,
                "Sprite num out of bounds.\nOffending value: %d\n", num);
      }
      else{//let the user handle the error
        SET_FLAG(system->cpu.status, BIT_X);
        system->cpu.curVal = 0;
      }
      return 1;
    }
    return 0;
  }

  static void _loadSpr(ASMSys *system){
    //going to return if op was successful
    system->cpu.status = system->cpu.curVal = 0;

    //grab arguments
    bagWord sprite_num = (bagWord)(*system->cpu.arg[0]);
    char *file = (char*)system->cpu.arg[1];

    //make sure a valid sprite number is provided
    if(_checkSprNum(system, sprite_num)) return;
    system->cpu.curVal = 1;//successful load

  	char newLine[MAX_PATH];
    BASM_CheckPathLocality(system, file, newLine, MAX_PATH);
  	if(BAG_Display_LoadObj(newLine, system->video.sprite[sprite_num]) != ERR_NONE){
      if(GET_FLAG(system->flags, ASM_AUTOERROR))
  		  errorMsg(system, system->prgm.pos, "Missing sprite: %s\n", newLine);//eventually remove this
      else{//let the user handle the error
        SET_FLAG(system->cpu.status, BIT_X);
        system->cpu.curVal = 0;
      }
    }
    return;
  }

//create a blank sprite
static void _createSpr(ASMSys *system){
    //sprite id, wd, ht
    bagDWord spriteID = *system->cpu.arg[0],
                wd = *system->cpu.arg[1],
                ht = *system->cpu.arg[2];
    
    
    BAG_Display_CreateObj(system->video.sprite[spriteID], 16, wd, ht, wd, ht);
    return;
}

//store pointer to a specified sprites graphics buffer
static void _getSprGfxBuf(ASMSys *system){
    //register, sprite id
    unsigned short *buf = BAG_Display_GetGfxBuf(system->video.sprite[*system->cpu.arg[1]]);
    *system->cpu.arg[0] = (bagAddrPtr)buf;
    
}



  static void _cloneSpr(ASMSys *system){
    //dest sprite, src sprite
    system->cpu.status = system->cpu.curVal = 0;

    //grab arguments
    bagWord destSprite = (bagWord)(*system->cpu.arg[0]),
        srcSprite  = (bagWord)(*system->cpu.arg[1]);

    if(_checkSprNum(system, destSprite) || _checkSprNum(system, srcSprite)) return;
    BAG_Display_CopyObj(system->video.sprite[destSprite], system->video.sprite[srcSprite]);
    return;
  }

  //set a sprites frame width and height
  static void _setSprFrameDim(ASMSys *system){
    system->cpu.status = system->cpu.curVal = 0;
    //sprite num, width, height
    bagWord sprite_num = (bagWord)(*system->cpu.arg[0]),
        newWd = (bagWord)(*system->cpu.arg[1]),
        newHt = (bagWord)(*system->cpu.arg[2]);

    if(_checkSprNum(system, sprite_num)) return;

    if(newWd > 0 && newHt > 0){
      BAG_Display_SetGfxFrameDim(system->video.sprite[sprite_num], newWd, newHt);
      system->cpu.curVal = 1;
    }
    return;
  }

  static void _setSprFrame(ASMSys *system){
    system->cpu.status = system->cpu.curVal = 0;
    //sprite num, axis, frame number
    bagWord sprite_num = (bagWord)(*system->cpu.arg[0]),
        axis = (bagWord)(*system->cpu.arg[1]),
        frame = (bagWord)(*system->cpu.arg[2]);

    if(_checkSprNum(system, sprite_num)) return;

    if(frame > 0){
      BAG_Display_SetObjFrame(system->video.sprite[sprite_num], axis, frame);
      system->cpu.curVal = 1;
    }
    return;
  }


  static void _setSprXY(ASMSys *system){
    //sprite num, x pos, y pos
    bagWord sprite_num = (bagWord)(*system->cpu.arg[0]),
        xpos = (bagWord)(*system->cpu.arg[1]),
        ypos = (bagWord)(*system->cpu.arg[2]);

    if(_checkSprNum(system, sprite_num)) return;
    BAG_Display_SetGfxBlitXY(system->video.sprite[sprite_num], xpos, ypos);
    return;
  }


  static void _deleteSpr(ASMSys *system){
    //going to return if op was successful
    system->cpu.status = system->cpu.curVal = 0;

    //arguments
    bagWord sprite_num = (bagWord)(*system->cpu.arg[0]);

    //function
    if(_checkSprNum(system, sprite_num)) return;

    BAG_Display_DeleteObj(system->video.sprite[sprite_num]);
    system->cpu.curVal = 1;
    return;
  }


  static void _drawSpr(ASMSys *system){
    //dest, dest wd, dest ht, sprite id
      
      
    //going to return if op was successful
    system->cpu.status = system->cpu.curVal = 0;

    //grab arguments first
    unsigned short *dest = (unsigned short*)(*system->cpu.arg[0]);
    bagWord wd = (bagWord)(*system->cpu.arg[1]),
            ht = (bagWord)(*system->cpu.arg[2]),
            id = (bagWord)(*system->cpu.arg[3]);

    //function
    if(_checkSprNum(system, id)) return;

    vidQNode temp = {id, QUEUE_TYPE_SPR, 0, wd, ht, (void*)dest};
    videoQueue_Add(system->video.gfxQue, &temp, system->video.sprite[id]);
    system->cpu.curVal = 1;
    return;
  }


  static void _setSprAlpha(ASMSys *system){

    //going to return if op was successful
    system->cpu.status = system->cpu.curVal = 0;

    //grab arguments first
    bagWord sprite_num = (bagWord)(*system->cpu.arg[0]),
        alpha = (bagWord)(*system->cpu.arg[1]);

    if(_checkSprNum(system, sprite_num)) return;

    if(alpha >= 0 && alpha <= 255)
      BAG_Display_SetGfxAlpha(system->video.sprite[sprite_num], alpha);
  }



#endif


/*==============================================
Backgrounds
==============================================*/

#if defined(BASM_ALLOW_BACKGROUNDS)
  static int _checkBgNum(ASMSys *system, int num){
    if(num < 0 || num > BASM_MAX_BACKGROUNDS){
      if(GET_FLAG(system->flags, ASM_AUTOERROR)){
        errorMsg(system, system->prgm.pos,
                "Background num out of bounds.\nOffending value: %d\n", num);
      } else {
        SET_FLAG(system->cpu.status, BIT_X);
        system->cpu.curVal = 0;
      }
      return 1;
    }
    return 0;
  }

  static void _loadBG(ASMSys *system){
    //arguments
    bagWord bg_num = (bagWord)(*system->cpu.arg[0]);
    char *file = (char*)system->cpu.arg[1];

    //function
    if(_checkBgNum(system, bg_num)) return;


    system->cpu.curVal = 1;
    char newLine[MAX_PATH];
    BASM_CheckPathLocality(system, file, newLine, MAX_PATH);
    if(BAG_TileBG_LoadBG(newLine, system->video.background[bg_num]) != ERR_NONE){
      if(GET_FLAG(system->flags, ASM_AUTOERROR)){
        errorMsg(system, system->prgm.pos, "Missing background: %s\n", newLine);
      }
      else {
        SET_FLAG(system->cpu.status, BIT_X);
        system->cpu.curVal = 0;
      }
    }

    if(system->cpu.curVal){
      TiledBG_t *bg = system->video.background[bg_num];
      BAG_TileBG_SetProperties(bg,
      system->settings.screenHeight >> bg->divY,
      system->settings.screenWidth >> bg->divX, 0, 0);
    }

    return;
  }

  static void _deleteBG(ASMSys *system){
    //going to return if op was successful
    system->cpu.status = system->cpu.curVal = 0;

    //arguments
    bagWord bg_num = (bagWord)(*system->cpu.arg[0]);

    //function
    if(_checkBgNum(system, bg_num)) return;
    BAG_TileBG_DeleteBG(system->video.background[bg_num]);
    system->cpu.curVal = 1;
    return;
  }

  static void _drawBG(ASMSys *system){
    //going to return if op was successful
    system->cpu.status = system->cpu.curVal = 0;

    //arguments
    char screen = (*system->cpu.arg[0]);
    bagWord bg_num = (bagWord)(*system->cpu.arg[1]);

    //function
    if(_checkBgNum(system, bg_num)) return;

    vidQNode temp = {bg_num, QUEUE_TYPE_BG, screen};
    videoQueue_Add(system->video.gfxQue, &temp, NULL);
    system->cpu.curVal = 1;
    return;
  }

#endif


#if defined(BASM_ALLOW_FONTS)
  static int _checkFontNum(ASMSys *system, int num){
    if(num < 0 || num > BASM_MAX_FONTS){
      if(GET_FLAG(system->flags, ASM_AUTOERROR)){
        errorMsg(system, system->prgm.pos,
                "Font slot out of bounds.\nOffending value: %d\n", num);
      } else {
        SET_FLAG(system->cpu.status, BIT_X);
        system->cpu.curVal = 0;
      }
      return 1;
    }
    return 0;
  }

  static int _checkTboxNum(ASMSys *system, int num){
    if(num < 0 || num > BASM_MAX_TEXTBOXS){
      if(GET_FLAG(system->flags, ASM_AUTOERROR)){
        errorMsg(system, system->prgm.pos,
                "Textbox slot out of bounds.\nOffending value: %d\n", num);
      } else {
        SET_FLAG(system->cpu.status, BIT_X);
        system->cpu.curVal = 0;
      }
      return 1;
    }
    return 0;
  }

  static void _loadFont(ASMSys *system){
    //arguments
    int slot = (*system->cpu.arg[0]);
    char *file = (char*)system->cpu.arg[1];

    if(_checkFontNum(system, slot)) return;

    char newLine[MAX_PATH];
    BASM_CheckPathLocality(system, file, newLine, MAX_PATH);
    if(BAG_Font_Load(newLine, system->video.font[slot]) != ERR_NONE){
      if(GET_FLAG(system->flags, ASM_AUTOERROR)){
        errorMsg(system, system->prgm.pos, "Error loading font\n", slot);
        return;
      } else {
        SET_FLAG(system->cpu.status, BIT_X);
        system->cpu.curVal = 0;
      }
    }
    return;
  }

  static void _fontCol(ASMSys *system){
    int slot = (*system->cpu.arg[0]);
    unsigned short color = (u16)(*system->cpu.arg[1]);

    if(_checkFontNum(system, slot)) return;
    BAG_Font_SetFontColor(system->video.font[slot], color);
    return;
  }

  static void _fontPrint(ASMSys *system){
    //arguments
    //screen, font slot, textbox slot, text

    int screen = (*system->cpu.arg[0]);
    int fontNum = (*system->cpu.arg[1]);
    int boxNum = (*system->cpu.arg[2]);
    char *text = (char*) system->cpu.arg[3];

    if(_checkTboxNum(system, boxNum)) return;

    fontData *data = calloc(1, sizeof(fontData));
    if(!data){
      BASM_DBGPRINT("Error allocating font data\n");
      return;
    }

    wString_new(&data->string);
    data->string.printf(&data->string, text);

    //ToDo: test text box stuff to be replaced with the textbox slot system
     /*TextBox_t tempBox = {
        30,30,//x1, y1
        200, SCREEN_HEIGHT,//x2, y2
        ALIGN_CENTER,//text alignment (ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER)
        WRAP_WORD,//text wrapping mode (WRAP_NONE, WRAP_LETTER, WRAP_WORD)
        TBOX_WHOLESTR,//print whole string, otherwise specify a numerical number of characters to print
    };*/

    data->box = system->video.fontBox[boxNum];
    vidQNode temp = {fontNum, QUEUE_TYPE_FNT, screen};
    videoQueue_Add(system->video.gfxQue, &temp, data);
    return;
  }



  static void _textBoxProperties(ASMSys *system){
    //tboxNum, x1, y1, x2, y2, alignment, wrap, characters
    int tbox = *system->cpu.arg[0],
        x1   = *system->cpu.arg[1],
        y1   = *system->cpu.arg[2],
        x2   = *system->cpu.arg[3],
        y2   = *system->cpu.arg[4],
        alignment = *system->cpu.arg[5],
        wrap = *system->cpu.arg[6],
        letterCount = *system->cpu.arg[7];

    if(_checkTboxNum(system, tbox)) return;

    TextBox_t *box = &system->video.fontBox[tbox];

    BAG_Font_TextBoxSetLimits(box, x1, y1, x2, y2);
    BAG_Font_TextBoxAlign(box, alignment);
    BAG_Font_TextBoxWrap(box, wrap);
    BAG_Font_TextBoxSetCharLim(box, letterCount);

    return;
  }

  //ToDo: Add text box control operations
          //x and y locations
          //text alignment
          //letter output (maybe)
#endif


static void _drawAll(ASMSys *system, videoQueue *qSys){
  //while there are things to draw
  ASMVideo *vid = &system->video;
  while(qSys->curQue > 0){

    vidQNode *curNode = videoQueue_Pop(qSys);
    if(!curNode) break;

    unsigned short *dest = *system->video.outScreen[curNode->screen];
    int i = curNode->updateSlot;
    switch(curNode->type){

      #if defined(BASM_ALLOW_SPRITES)
        case QUEUE_TYPE_SPR:
          if(curNode->data) {
            //BAG_Display_SetGfxBlitXY(&curNode->data, system->video.spX[i], system->video.spY[i]);
            BAG_Display_DrawObjSlowEx((GFXObj_t*)curNode->data, curNode->dest, curNode->destWd, curNode->destHt);
            //free allocated gfx copy
            free(curNode->data);
            curNode->data = NULL;
          }

        break;
      #endif

      #if defined(BASM_ALLOW_BACKGROUNDS)
        case QUEUE_TYPE_BG:
          BAG_TileBG_DrawBGEx(dest, vid->background[i], vid->bgX[i], vid->bgY[i],
                              system->settings.screenWidth, system->settings.screenHeight);
        break;
      #endif

      #if defined(BASM_ALLOW_FONTS)
        case QUEUE_TYPE_FNT:{
          fontData *temp = (fontData *)curNode->data;
          BAG_Font_Print2(dest, (TextBox_t *)&temp->box, (wString *)&temp->string, system->video.font[i]);
          temp->string.del(&temp->string);
        }break;
      #endif
    }
    //free memory used by the node
    free(curNode);
  }
  return;
}

 static void _flip(ASMSys *system){
  //grab arguments
  bagWord screen = (bagWord)(*system->cpu.arg[0]);

  system->video.update = 1;
  _drawAll(system, system->video.gfxQue);
  //finally draw the screen
  if(!system->settings.passive)
    BAG_Display_DrawObjFast(system->video.screen[screen], *screen_addr[screen], system->settings.x, system->settings.y);

  //if script is running in passive mode, user can't force system features
  if(!system->settings.passive)
  	   ds2_flipScreen(2 - screen, system->settings.flipMode);

  return;
}


static void _vsync(ASMSys *system){
  //if script running in passive mode, user can't force a vsync
  if(!system->settings.passive)
  	   BAG_Update();

  return;
}

#if defined(BASM_COMPILE_ADVANCED_VIDEO)
    static void ASM_ResizeVideo(ASMSys *system){
      //system initialization
      system->cpu.status = 0;

      //arguments
      bagWord newWd = (bagWord)(*system->cpu.arg[0]),
              newHt = (bagWord)(*system->cpu.arg[1]);


      //function
      system->settings.screenWidth = newWd;
      system->settings.screenHeight = newHt;
      int val = ASM_screenInit(system);

      //return value
      system->cpu.curVal = val;
      return;
    }
#endif




/**************************************************************************************
Mapping operations to functions

**************************************************************************************/

#ifdef COLLISION_DETECTION
static void chkcol(ASMSys *system){
    //check collisions between two sprites
    system->cpu.status = 0;//reset cpu status each operation
    system->cpu.curVal = cpu_checkCollision(system, (bagWord)(*system->cpu.arg[0]), (bagWord)(*system->cpu.arg[1]));
    return;
}
#endif//collision_detection



static void ASM_initVidOps(ASMSys *system){
    system->global->vidOpInit++;
    if(system->global->vidOpInit > 1) return;

//New video api stuff
    ASM_setCpuOpsEx(system->global, "RGB", 4, &rgb);
    ASM_setCpuOpsEx(system->global, "GETLCD", 2, &_getLCDBuffer);
    ASM_setCpuOpsEx(system->global, "GFXMEMSET", 3, &_gfxMemSet);
    ASM_setCpuOpsEx(system->global, "GFXSETPIX", 5, &_gfxSetPix);
    ASM_setCpuOpsEx(system->global, "GFXSETRGB", 8, &_gfxSetPixRGB);
    ASM_setCpuOpsEx(system->global, "GFXGETPIX", 5, &_gfxGetPix);
    ASM_setCpuOpsEx(system->global, "GFXDRWLINE", 9, _gfxDrwLine);
    ASM_setCpuOpsEx(system->global, "GFXDRWREC", 8, &_gfxDrwRec);
    
    
    
    ASM_setCpuOpsEx(system->global, "FLIP", 1, &_flip);
    ASM_setCpuOpsEx(system->global, "VSYNC", 0, &_vsync);
    
    
#if defined(BASM_ALLOW_SPRITES)
    ASM_setCpuOpsEx(system->global, "BLANKSPR", 3, &_createSpr);
    ASM_setCpuOpsEx(system->global, "GETSPRGFX", 2, &_getSprGfxBuf);
    
    ASM_setCpuOpsEx(system->global, "LDSPR", 2, &_loadSpr);
    ASM_setCpuOpsEx(system->global, "DELSPR", 1, &_deleteSpr);
    ASM_setCpuOpsEx(system->global, "SPRFRAMEDIM", 3, &_setSprFrameDim);
    ASM_setCpuOpsEx(system->global, "SPRFRAME", 3, &_setSprFrame);
    ASM_setCpuOpsEx(system->global, "CLONESPR", 2, &_cloneSpr);
    ASM_setCpuOpsEx(system->global, "SETSPRXY", 3, &_setSprXY);
    ASM_setCpuOpsEx(system->global, "SETSPRALPHA", 2, &_setSprAlpha);
    ASM_setCpuOpsEx(system->global, "DRWSPR", 2, &_drawSpr);
#endif
    
    
  #if defined(COLLISION_DETECTION)
      ASM_setCpuOpsEx(system->global, "CHKCOL", 2, &chkcol);
  #endif

  #if defined(BASM_COMPILE_ADVANCED_VIDEO)
    ASM_setCpuOpsEx(system->global, "SETVIDSIZE", 2, &ASM_ResizeVideo);
  #endif


  #if defined(BASM_ALLOW_BACKGROUNDS)
      ASM_setCpuOpsEx(system->global, "DELBG", 1, &_deleteBG);
      ASM_setCpuOpsEx(system->global, "LDBG", 2, &_loadBG);
      ASM_setCpuOpsEx(system->global, "DRWBG", 2, &_drawBG);
  #endif

  #if defined(BASM_ALLOW_FONTS)
      ASM_setCpuOpsEx(system->global, "LDFNT", 2, &_loadFont);
      ASM_setCpuOpsEx(system->global, "FNTPRINT", 4, &_fontPrint);
      ASM_setCpuOpsEx(system->global, "FNTSETCOL", 2, &_fontCol);
      ASM_setCpuOpsEx(system->global, "TBOXINIT", 8, &_textBoxProperties);

  #endif

  return;
}

int ASM_videoInit(ASMSys *system){
  if(!system->settings.screenWidth)
    system->settings.screenWidth = SCREEN_WIDTH;

  if(!system->settings.screenHeight)
    system->settings.screenHeight = SCREEN_HEIGHT;

  ASM_initVidOps(system);

  if(!initGfxQue(system))
    return 0;

  for(int i = 0; i < 1; i++)
    system->video.screen[i] = NULL;

  //assign default output devices
  system->video.outScreen[0] = (void *)&down_screen_addr;
  system->video.outScreen[1] = (void *)&up_screen_addr;

  //if program not running as a script, it needs to make its own screens
  if(!system->settings.passive){
    if(!ASM_screenInit(system))
        return 0;
  }

  #if defined(DBGTEXT)
      if(system->settings.debugTxt)
          printFunction(system,"Screens initialized\nStarting sprites...\n");
  #endif

  #if defined(BASM_ALLOW_SPRITES)
      for(int i = 0; i < BASM_MAX_SPRITES; i++)
        system->video.sprite[i] = NULL;

      if(!initSprites(system))
          return 0;
  #endif

  #if defined(DBGTEXT)
      if(system->settings.debugTxt)
          printFunction(system,"Sprites initialized\nStarting backgrounds...\n");
  #endif

  #if defined(BASM_ALLOW_BACKGROUNDS)
      for(int i = 0; i < BASM_MAX_BACKGROUNDS; i++)
          system->video.background[i] = NULL;

      if(!initBackgrounds(system))
          return 0;
  #endif
  #if defined(DBGTEXT)
      if(system->settings.debugTxt)
          printFunction(system,"Backgrounds initialized\nStarting fonts...\n");
  #endif

  #if defined(BASM_ALLOW_FONTS)
      for(int i = 0; i< BASM_MAX_FONTS; i++)
        system->video.font[i] = NULL;

      if(!initFonts(system))
        return 0;
  #endif
    #if defined(DBGTEXT)
      if(system->settings.debugTxt)
          printFunction(system,"Sprites initialized\nGraphics Done!\n");
  #endif

  return 1;
}



#endif//BASM_COMPILE_VIDEO
