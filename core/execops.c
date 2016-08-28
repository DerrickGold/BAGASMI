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

#include "bagasmi.h"

//from bagasmi.c
extern void setCpuOps(ASMGlobal *global, const char *name, bagWord opVal);
extern void ASM_setCpuOpsEx(ASMGlobal *global, const char *name, char params, void (*function)(ASMSys *system));
extern bagAddrPtr readAddress(unsigned char *buf);


//Ring buffers used in JUMP statements
void ringBuf_init(RingBuf *buffers, int bufCount){
    buffers->cur = buffers->start = -1;
    buffers->end = buffers->total = bufCount - 1;
    return;
}


//circular buffers for jump backs and such
static bagWord ringBuf_Next(RingBuf *buffers){
    buffers->cur++;
    //update buffer positions
    if(buffers->cur > buffers->end){
        buffers->end = (buffers->start += (buffers->start == -1));

        buffers->start++;
        if(buffers->start > buffers->total)
            buffers->start = 0;

        buffers->cur = buffers->end;
    }
    //printf("ring pos:%d\n", buffers->cur);
    return buffers->cur;
}

static int ringBuf_Prev(RingBuf *buffers){
    buffers->cur--;
    if(buffers->cur < 0)
        buffers->cur = buffers->total;

    //we have looped around to the begining
    if(buffers->cur == buffers->end){
        buffers->start = buffers->end;
        buffers->end--;
        if(buffers->end < 0)
            buffers->end = buffers->total;
        buffers->cur = buffers->end;
    }
    //printf("ring pos:%d\n", buffers->cur);
    return buffers->cur;
}


/*==================================================================
    Helper functions
==================================================================*/
static inline bagDWord ASM_loadWordAdr(bagAddrPtr *memory){
    //printf("reading address: %u\n", memory);
    #if defined(USE_64BIT)
        return READ_64BIT(memory);
    #else
        return READ_32BIT(memory);
    #endif
}

static inline bagDWord ASM_loadShortAdr(bagShort *memory){
    return READ_16BIT(memory);
}

static inline bagDWord ASM_loadByteAdr(bagMemInt *memory){
    return READ_8BIT(memory);
}


static inline void ASM_storeByteAdr(bagMemInt *memory, char value){
    WRITE_8BIT(memory, value);
    return;
}

static inline void ASM_storeShortAdr(bagShort *memory, short value){
    WRITE_16BIT(memory, value);
    return;
}


static inline void ASM_storeWordAdr(bagAddrPtr *memory, bagDWord value){
    #if defined(USE_64BIT)
        WRITE_64BIT(memory, value);
    #else
        WRITE_32BIT(memory, value);
    #endif
    return;
}

/*============================
 Misc operations
 ============================*/
static void nop(ASMSys *system){
    return;
}

static void halt(ASMSys *system){
    SET_FLAG(system->cpu.status, BIT_HALT);
    system->cpu.exit = BASM_EXIT_CLEAN;
    return;
}

static void yield(ASMSys *system){
    SET_FLAG(system->cpu.status, BIT_YIELD);
    system->cpu.exit = BASM_EXIT_SUSPEND;
    return;
}


static void clear(ASMSys *system){
    //clr, clrh, clrc
    (*system->cpu.arg[0]) = 0;
    return;
}


/*==================================================================
    Standard Operations
==================================================================*/
#define CPU_OP_INIT {system->cpu.status = system->cpu.curVal = 0;}


#if defined(PLATFORM_DS2)
    const short CPU_LEVELS[19] = {60,120,120,144,
                               192,204,240,264,
                               288,300,336,360,
                               384,396,404,420,
                               438,444,456 };

    static inline int _cpuCheckFreq(ASMSys *system, int cpuFreq){
        if(cpuFreq < 0 || cpuFreq > 18){
            if(GET_FLAG(system->flags, ASM_AUTOERROR)){
                errorMsg(system, system->prgm.pos,
                  "Invalid CPU Frequency.\nOffending value: %d\n", cpuFreq);
            } else {
                SET_FLAG(system->cpu.status, BIT_X);
                system->cpu.curVal = 0;
            }
            return 1;
        }
        return 0;
    }

    static void setCPUFreq(ASMSys *system){
        //arguments
        //frequency value (0 - 19)
        int cpuFreq = (int)(*system->cpu.arg[0]);

        if(_cpuCheckFreq(system, cpuFreq)) return;
        else{
            system->cpu.freq = cpuFreq;
            ds2_setCPULevel(cpuFreq);
            mdelay(12);
        }
        return;
    }

    static void getCPUFreq(ASMSys *system){
        //arguments
        //storage
        *system->cpu.arg[0] = CPU_LEVELS[system->cpu.freq];
    }


#endif



static void getARGV(ASMSys *system){
    bagDWord arg = *system->cpu.arg[1];
    if(arg < system->passed_args.argc){
        bagMemInt * mem = &system->memory.byte[arg * sizeof(bagAddrPtr)];
        strcpy((char*)system->cpu.arg[0], (char*)readAddress(mem));
        system->cpu.curVal = 1;
    }
    else{//no arg value, so set it to default 1
        strcpy((char*)system->cpu.arg[0], "1");
        system->cpu.curVal = 0;
    }
    return;
}


/*============================
Jump operations
============================*/
//first argument is label to jump to
//all extra arguments are passed to the a registers (a0 - a3)
//program position is then saved to register $ra
static void call(ASMSys *system){
    //first store the current position (+1) so we can
    //exit out of the function call later on
    system->cpu.reg[REG_RA] = system->prgm.pos;
    
    //next push all arguments to a registers
    //loop unrolled to remove unecessary overhead
    system->cpu.reg[REG_A0] = (*system->cpu.arg[1]);
    system->cpu.reg[REG_A1] = (*system->cpu.arg[2]);
    system->cpu.reg[REG_A2] = (*system->cpu.arg[3]);
    system->cpu.reg[REG_A3] = (*system->cpu.arg[4]);
    
    //jump to position
    system->prgm.pos = ASM_loadWordAdr((bagAddrPtr*)system->cpu.arg[0]);
    return;
}

static void jump(ASMSys *system){
    if(system->cpu.arg[1]){
        int pos = ringBuf_Next(&system->prgm.jmpBackPos);
        system->prgm.preJumpPos[pos] = system->prgm.pos;
    }
    system->prgm.pos = ASM_loadWordAdr((bagAddrPtr*)system->cpu.arg[0]);
    //printf("Jumping to line: %d\n", system->prgm.pos);
    return;
}

//jump register
static void jumpr(ASMSys *system) {
    if(system->cpu.arg[1]){
        int pos = ringBuf_Next(&system->prgm.jmpBackPos);
        system->prgm.preJumpPos[pos] = system->prgm.pos;
    }
    system->prgm.pos = (*system->cpu.arg[0]);
}

static void jmpbk(ASMSys *system){
    system->prgm.pos = system->prgm.preJumpPos[system->prgm.jmpBackPos.cur];
    if(ringBuf_Prev(&system->prgm.jmpBackPos) <= -1)
        system->prgm.jmpBackPos.cur = 0;
    return;
}

static void jmpc(ASMSys *system){//jump collision
    if(GET_FLAG(system->cpu.status, BIT_COLLISION)){
        jump(system);
        CPU_OP_INIT;
    }

    return;
}

static void jmpn(ASMSys *system){//jump negative
    if(GET_FLAG(system->cpu.status, BIT_N)){
        jump(system);
        CPU_OP_INIT;
    }

    return;
}

static void jmpp(ASMSys *system){//jump positive
    if(GET_FLAG(system->cpu.status, BIT_P)){
        jump(system);
        CPU_OP_INIT;
    }

    return;
}

static void jmpx(ASMSys *system){//jump error
    if(GET_FLAG(system->cpu.status, BIT_X)){
        jump(system);
        CPU_OP_INIT;
    }

    return;
}

//swaps arguments 1 and 2, with 3 and 4 and executes a jump
void _equalityJump(ASMSys *system){
    //rearrange some arguments
    system->cpu.arg[0] = system->cpu.arg[2];
    system->cpu.arg[1] = system->cpu.arg[3];
    system->cpu.arg[2] = system->cpu.arg[4];
    jump(system);

    return;
}

//equal to (rhs, lhs, label, disable jmpbk)
static void eq(ASMSys *system){
    if((bagWord)(*system->cpu.arg[0]) == (bagWord)(*system->cpu.arg[1])){
        _equalityJump(system);
    }
    return;
}

//greater than (rhs, lhs, label, disable jmpbk)
static void gt(ASMSys *system){
    if((bagWord)(*system->cpu.arg[0]) > (bagWord)(*system->cpu.arg[1])){
        _equalityJump(system);
    }
    return;
}

//greater or equal than (rhs, lhs, label, disable jmpbk)
static void gte(ASMSys *system){
    if((bagWord)(*system->cpu.arg[0]) >= (bagWord)(*system->cpu.arg[1])){
        _equalityJump(system);
    }
    return;
}

//less than (rhs, lhs, label, disable jmpbk)
static void lt(ASMSys *system){
    if((bagWord)(*system->cpu.arg[0]) < (bagWord)(*system->cpu.arg[1])){
        _equalityJump(system);
    }
    return;
}

//less or equal than (rhs, lhs, label, disable jmpbk)
static void lte(ASMSys *system){
    if((bagWord)(*system->cpu.arg[0]) <= (bagWord)(*system->cpu.arg[1])){
        _equalityJump(system);
    }
    return;
}


/*============================
Math operations
============================*/
static void _abs(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = (int)abs((int)*system->cpu.arg[1]);
    return;
}

static void add(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = (bagDWord)(*system->cpu.arg[1]) + (bagDWord)(*system->cpu.arg[2]);
    return;
}

static void addu(ASMSys *system){
    bagUDWord result = ((bagUDWord)(*system->cpu.arg[1]) + (bagUDWord)(*system->cpu.arg[2]));
    system->cpu.curVal = (*system->cpu.arg[0]) = result;
    return;
}

static void sub(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = (bagDWord)(*system->cpu.arg[1]) - (bagDWord)(*system->cpu.arg[2]);
    return;
}

static void subu(ASMSys *system){
    bagUDWord result = ((bagUDWord)(*system->cpu.arg[1]) - (bagUDWord)(*system->cpu.arg[2]));
    system->cpu.curVal = (*system->cpu.arg[0]) = result;
    return;
}


static void mul(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = (bagDWord)(*system->cpu.arg[1]) * (bagDWord)(*system->cpu.arg[2]);
    return;
}

static void mulu(ASMSys *system){
    bagUDWord result = ((bagUDWord)(*system->cpu.arg[1]) * (bagUDWord)(*system->cpu.arg[2]));
    system->cpu.curVal = (*system->cpu.arg[0]) = result;
    return;
}

static void _div(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = (bagDWord)(*system->cpu.arg[1]) / ((bagDWord)*system->cpu.arg[2]);
    return;
}

static void _divu(ASMSys *system){
    bagUDWord result = ((bagUDWord)(*system->cpu.arg[1]) * (bagUDWord)(*system->cpu.arg[2]));
    system->cpu.curVal = (*system->cpu.arg[0]) = result;
    return;
}

static void rem(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = (*system->cpu.arg[1]) % (*system->cpu.arg[2]);
    return;
}

static void incr(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0])++;
    return;
}

static void decr(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0])--;
    return;
}

static void randm(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = (rand()%(*system->cpu.arg[1]));
    return;
}

static void shl(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = (*system->cpu.arg[1]) << (*system->cpu.arg[2]);
    return;
}

static void shlu(ASMSys *system){
    bagUDWord result = ((bagUDWord)(*system->cpu.arg[1]) << (bagUDWord)(*system->cpu.arg[2]));
    system->cpu.curVal = (*system->cpu.arg[0]) = result;
    return;
}

static void shr(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = (*system->cpu.arg[1]) >> (*system->cpu.arg[2]);
    return;
}

static void shru(ASMSys *system){
    bagUDWord result = ((bagUDWord)(*system->cpu.arg[1]) >> (bagUDWord)(*system->cpu.arg[2]));
    system->cpu.curVal = (*system->cpu.arg[0]) = result;
    return;
}

static void and(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = (*system->cpu.arg[1]) & (*system->cpu.arg[2]);
    return;
}

static void or(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = (*system->cpu.arg[1]) | (*system->cpu.arg[2]);
    return;
}

static void xor(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = (*system->cpu.arg[1]) ^ (*system->cpu.arg[2]);
}

static void compl(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = ~(*system->cpu.arg[1]);
}

static void getary2d(ASMSys *system){
    //get a slot in a 2d array
    //register = x + y * width
    system->cpu.curVal = (*system->cpu.arg[0]) = ((*system->cpu.arg[1]) + (*system->cpu.arg[2]) * (*system->cpu.arg[3]));
    return;
}

static void getary3d(ASMSys *system){
    //get slot in a 3d array
    //register = x * len * wid + y * wid + z
    //register, x, y, z, len, width
    system->cpu.curVal = (*system->cpu.arg[0]) = ( (*system->cpu.arg[1]) * (*system->cpu.arg[4]) * (*system->cpu.arg[5]) +
                                                    (*system->cpu.arg[2]) * (*system->cpu.arg[5]) + (*system->cpu.arg[3]));
    return;
}


/*============================
Memory operations
============================*/

//deprecate these functions for better ones
static void ldary(ASMSys *system){
    //register,array,index
    system->cpu.curVal = (*system->cpu.arg[0]) =
                            ASM_loadWordAdr((bagAddrPtr*)system->cpu.arg[1] + (sizeof(bagDWord) * (*system->cpu.arg[2])));
    return;
}

static void ldaryh(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) =
                         ASM_loadShortAdr((bagShort*)system->cpu.arg[1] + (sizeof(short) * (*system->cpu.arg[2])));
    return;
}

static void ldaryc(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) =
                         ASM_loadByteAdr((bagMemInt*)system->cpu.arg[1] + (sizeof(char) * (*system->cpu.arg[2])));
    return;
}

static void stary(ASMSys *system){
    //register,array,index
    ASM_storeWordAdr((bagAddrPtr*)system->cpu.arg[1] + (sizeof(bagDWord) * (*system->cpu.arg[2])),
                     (*system->cpu.arg[0]));
    return;
}

static void staryh(ASMSys *system){
    ASM_storeShortAdr((bagShort*)system->cpu.arg[1] + (sizeof(bagShort) * (*system->cpu.arg[2])),
                      (*system->cpu.arg[0]));
    return;
}

static void staryc(ASMSys *system){
    ASM_storeByteAdr((bagMemInt*)system->cpu.arg[1] + (sizeof(char) * (*system->cpu.arg[2])),
                     (*system->cpu.arg[0]));
    return;
}



//macro for
//ldaddr reg, array
//add reg, reg, index
static void laary(ASMSys *system) {
    //register, array, index
    bagDWord *array = (bagDWord*)system->cpu.arg[1];
    unsigned int index = (unsigned int)*system->cpu.arg[2];

    *system->cpu.arg[0] = (bagDWord)&array[index];
    
    
    //(*system->cpu.arg[0]) = (bagUDWord)((bagMemInt*)system->cpu.arg[1] + ((*system->cpu.arg[2])<<2));
    return;
}

static void laaryh(ASMSys *system) {
    //register, array, index
    //printf("base addr: %x\n", *system->cpu.arg[1]);
    
    bagUShort *array = (bagUShort*)system->cpu.arg[1];
    unsigned int index = (unsigned int)*system->cpu.arg[2];
    *system->cpu.arg[0] = (bagAddrPtr)&array[index];

    //printf("loading addr: %d\n", *system->cpu.arg[0]);
    return;
}

static void laaryc(ASMSys *system) {
    //register, array, index
    (*system->cpu.arg[0]) = (bagUDWord)((bagMemInt*)system->cpu.arg[1] + ((*system->cpu.arg[2])));
    return;
}

static void load(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = ASM_loadWordAdr((bagAddrPtr*)system->cpu.arg[1]);
    return;
}

static void loadh(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = ASM_loadShortAdr((bagShort*)system->cpu.arg[1]);
    return;
}

static void loadc(ASMSys *system){
    system->cpu.curVal = (*system->cpu.arg[0]) = ASM_loadByteAdr((bagMemInt*)system->cpu.arg[1]);
    return;
}



static void store(ASMSys *system){
    ASM_storeWordAdr((bagAddrPtr*)system->cpu.arg[1], (*system->cpu.arg[0]));
    return;
}

static void storeh(ASMSys *system){
    ASM_storeShortAdr((bagShort*)system->cpu.arg[1], (*system->cpu.arg[0]));
    return;
}

static void storec(ASMSys *system){
    ASM_storeByteAdr((bagMemInt*)system->cpu.arg[1], (*system->cpu.arg[0]));
    return;
}

static void set(ASMSys *system){
    (*system->cpu.arg[0]) = (*system->cpu.arg[1]);
    return;
}

static void laddr(ASMSys *system){
    (*system->cpu.arg[0]) = (bagReg)system->cpu.arg[1];
    return;
}

#ifdef BASM_ENABLE_STACK

//push a word onto the stack
static void push(ASMSys *system) {
//    printf("current stack addr: %p\n", system->cpu.reg[REG_SP]);
    bagAddrPtr **sp = (bagAddrPtr**)&system->cpu.reg[REG_SP];
//    printf("Pushing to stack: %u\n", (*sp));
    (*sp) -= sizeof(bagUDWord);
    ASM_storeWordAdr(*sp, (*system->cpu.arg[0]));

    return;
}

//push all t registers to stack
static void pusht(ASMSys *system) {
    for(int i = REG_T0; i <= REG_T9; i++) {
        system->cpu.arg[0] = (bagAddrPtr*)&system->cpu.reg[i];
        push(system);
    }
}

//push all s registers to stack
static void pushs(ASMSys *system) {
    for(int i = REG_S0; i <= REG_S9; i++) {
        system->cpu.arg[0] = (bagAddrPtr*)&system->cpu.reg[i];
        push(system);
    }
}


static void pop(ASMSys *system) {
    bagAddrPtr **sp = (bagAddrPtr**)&system->cpu.reg[REG_SP];
    (*system->cpu.arg[0]) = ASM_loadWordAdr(*sp);
    (*sp) += sizeof(bagUDWord);

    return;
}

//pop off all t registers from stack
static void popt(ASMSys *system) {
    for(int i = REG_T9; i >= REG_T0; i--) {
        system->cpu.arg[0] = (bagAddrPtr*)&system->cpu.reg[i];
        pop(system);
    }
}

//pop off all s registers from stack
static void pops(ASMSys *system) {
    for(int i = REG_S9; i >= REG_S0; i--) {
        system->cpu.arg[0] = (bagAddrPtr*)&system->cpu.reg[i];
        pop(system);
    }
}

#endif

/*============================
Basic Output
============================*/
static void out(ASMSys *system){
    printFunction(system, "%d", (bagWord)(*system->cpu.arg[0]));

    //if doing a line by line interpreter, auto new line for printing
    if(GET_FLAG(system->flags, ASM_LINEBYLINE)) BASM_PRINT("\n");
    return;
}

static void outu(ASMSys *system) {
    printFunction(system, "%u", (bagUWord)(*system->cpu.arg[0]));
    if(GET_FLAG(system->flags, ASM_LINEBYLINE)) BASM_PRINT("\n");
    return;
}

static void outc(ASMSys *system){
    printFunction(system, "%c", (bagByte)(*system->cpu.arg[0]));

    //if doing a line by line interpreter, auto new line for printing
    if(GET_FLAG(system->flags, ASM_LINEBYLINE)) BASM_PRINT("\n");
    return;
}

static void print(ASMSys *system){
    printFunction((ASMSys*)system, (char*)system->cpu.arg[0] );

    //if doing a line by line interpreter, auto new line for printing
    if(GET_FLAG(system->flags, ASM_LINEBYLINE)) BASM_PRINT("\n");
    return;
}

static void clearConsole(ASMSys *system){
    #if defined(PLATFORM_DS2)
        ConsoleClr(1);
    #else
        printf ("\x1b[2J");
    #endif

    return;
}

//time
#if defined(ENABlE_TIME)
    extern int getTime(char *array);
    static void gettime(ASMSys *system){
        //gettime storage (array size char * 7)
        getTime((char*)system->cpu.arg[0]);
        return;
    }
#endif


/*============================
string stuff
============================*/
#if defined(BASM_ALLOW_STRINGS)
    static void _strncpy_(ASMSys *system){
        //dest,source,size
        system->cpu.curVal = (bagDWord)strncpy((char*)system->cpu.arg[0], (char*)system->cpu.arg[1], (*system->cpu.arg[2]));
        return;
    }

    static void _strncat_(ASMSys *system){
        //dest,source, size
        system->cpu.curVal = (bagDWord)strncat((char*)system->cpu.arg[0], (char*)system->cpu.arg[1], (*system->cpu.arg[2]));
        return;
    }

    static void _strlen_(ASMSys *system){
        //register, string
        system->cpu.curVal = (*system->cpu.arg[0]) = (bagWord)strlen((char*)system->cpu.arg[1]);
        return;
    }

    static void _strncmp_(ASMSys *system){
        //str1, str2, length
        system->cpu.curVal = (bagWord)strncmp((char*)system->cpu.arg[0], (char*)system->cpu.arg[1], (*system->cpu.arg[2]));
        return;
    }

    //array to integer
    static void _atoi_(ASMSys *system){
        //output register, input string
        system->cpu.curVal = (*system->cpu.arg[0]) = (bagWord)atoi((char*)system->cpu.arg[1]);
        return;
    }
#endif//BASM_ALLOW_STRINGS


/*==================================================================
    Loop Operations
==================================================================*/
#if defined(BASM_ALLOW_LOOPS)
    static void loopto(ASMSys *system){
        register bagDWord depth = (*system->cpu.arg[0]),
                          pos = depth<<1;
        system->loops.pos[pos] = system->prgm.pos;
        //set the to value
        system->loops.to[depth] = system->cpu.arg[2];
        //set increment value
        system->loops.increment[depth] = system->cpu.arg[3];
        //set the counter
        system->loops.counter[depth] = system->cpu.arg[1];
        return;
    }

    static void loopback(ASMSys *system){
        system->cpu.status = 0;
        register bagDWord depth = (*system->cpu.arg[0]),
                          pos = depth<<1;

        //jump back to loop start
        system->loops.pos[pos+1] = system->prgm.pos;
        system->prgm.pos = system->loops.pos[pos];

       (*system->loops.counter[depth]) += *system->loops.increment[depth];

        //check if the loop is incrementing or decrementing, then exit if checks are now false
        if((*system->loops.counter[depth]) > 0){
            if(((*system->loops.counter[depth])) >=  *system->loops.to[depth])
                system->prgm.pos = system->loops.pos[pos+1];
        }
        else if((bagDWord)(*system->loops.counter[depth]) < 0){
            if((*system->loops.counter[depth]) <=  *system->loops.to[depth])
                system->prgm.pos = system->loops.pos[pos+1];
        }
        //*system->loops.increment[depth] = count;
        return;
    }

    static void loopbreak(ASMSys *system){
        system->prgm.pos = system->loops.pos[((*system->cpu.arg[0])<<1)+1];
        return;
    }
#endif//BASM_ALLOW_LOOPS



/*==================================================================
    FAT Operations
==================================================================*/
#if defined(BASM_COMPILE_FAT)

static inline int _fatCheckHandle(ASMSys *system, int handle){
    if(handle < 0 || handle > BASM_FILEHANDLE_COUNT){
        if(GET_FLAG(system->flags, ASM_AUTOERROR)){
            errorMsg(system, system->prgm.pos,
              "File handle num out of bounds.\nOffending value: %d\n", handle);
        } else {
            SET_FLAG(system->cpu.status, BIT_X);
            system->cpu.curVal = 0;
        }
        return 1;
    }
    return 0;
}

//checks if a file handle has been opened or not
static inline int _fatCheckOpened(ASMSys *system, int handle){
    if(!system->fh[handle]){
        if(GET_FLAG(system->flags, ASM_AUTOERROR)){
            errorMsg(system, system->prgm.pos,
              "File not opened: %d\n", handle);
        } else {
            SET_FLAG(system->cpu.status, BIT_X);
            system->cpu.curVal = 0;
        }
        return 0;
    }
    return 1;
}

static void fatopen(ASMSys *system){
    //handle number, file path, mode
    system->cpu.status = system->cpu.curVal = 0;

    //check that the file handle selected is in range
    int handle = (int)(*system->cpu.arg[0]);
    if(_fatCheckHandle(system, handle)) return;

    //generate the file path
    char *path = (char*)system->cpu.arg[1];
    char newLine[MAX_PATH];

    //checks if we are using an absolute or local path
    BASM_CheckPathLocality(system, path, newLine, MAX_PATH);
    char *mode = (char*)system->cpu.arg[2];

    system->fh[handle] = fopen(newLine, mode);
    if(_fatCheckOpened(system, handle)){
        system->cpu.curVal = 1;
    }
    return;
}

static void fatread(ASMSys *system){
    //handle, buffer, count, size
    system->cpu.status = system->cpu.curVal = 0;

    int i = 0;
    int handle = (int)(*system->cpu.arg[i++]);
    void *buf = (void*)system->cpu.arg[i++];
    int count = (int)(*system->cpu.arg[i++]),
        size = (int)(*system->cpu.arg[i++]);

    if(_fatCheckHandle(system, handle)) return;

    else if(_fatCheckOpened(system, handle))
        system->cpu.curVal = (bagWord)fread(buf, count, size, system->fh[handle]);

    return;
}

static void fatwrite(ASMSys *system){
    //handle, buffer, count, size
    system->cpu.status = system->cpu.curVal = 0;

    int i = 0;
    int handle = (int)(*system->cpu.arg[i++]);
    void *buf = (void*)system->cpu.arg[i++];
    int count = (int)(*system->cpu.arg[i++]),
        size = (int)(*system->cpu.arg[i++]);

    if(_fatCheckHandle(system, handle)) return;

    else if(_fatCheckOpened(system, handle))
        system->cpu.curVal = (bagWord)fwrite(buf, count, size, system->fh[handle]);

    return;
}

static void fatgets(ASMSys *system){
    //handle, buffer, max size
    system->cpu.status = system->cpu.curVal = 0;

    int i = 0;
    int handle = (int)(*system->cpu.arg[i++]);
    void *buf = (void*)system->cpu.arg[i++];
    int maxSize = (int)(*system->cpu.arg[i++]);

    if(_fatCheckHandle(system, handle)) return;

    else if(_fatCheckOpened(system, handle))
        fgets(buf, maxSize, system->fh[handle]);

    return;
}

static void fatprint(ASMSys *system){
    //handle, str
    system->cpu.status = system->cpu.curVal = 0;

    int i = 0;
    int handle = (int)(*system->cpu.arg[i++]);
    char *str = (void*)system->cpu.arg[i++];

    if(_fatCheckHandle(system, handle)) return;

    else if(_fatCheckOpened(system, handle))
        system->cpu.curVal = fprintf(system->fh[handle], "%s", str);

    return;
}

static void fatclose(ASMSys *system){
    //handle
    int handle = (int)(*system->cpu.arg[0]);
    if(_fatCheckHandle(system, handle)) return;
    else if(_fatCheckOpened(system, handle))
        fclose(system->fh[handle]);
    system->fh[handle] = NULL;
    return;
}

static void fatflush(ASMSys *system){
    //handle
    int handle = (int)(*system->cpu.arg[0]);
    if(_fatCheckHandle(system, handle)) return;
    else if(_fatCheckOpened(system, handle))
        fflush(system->fh[handle]);
    return;
}

static void fatrewind(ASMSys *system){
    //handle
    int handle = (int)(*system->cpu.arg[0]);
    if(_fatCheckHandle(system, handle)) return;
    else if(_fatCheckOpened(system, handle))
        rewind(system->fh[handle]);
    return;
}

static void fatseek(ASMSys *system){
    system->cpu.status = system->cpu.curVal = 0;
    //handle, start, offset
    int handle = (int)(*system->cpu.arg[0]),
        start = (int)(*system->cpu.arg[1]),
        offset = (int)(*system->cpu.arg[2]);

    if(_fatCheckHandle(system, handle)) return;
    else if(_fatCheckOpened(system, handle))
        system->cpu.curVal = fseek(system->fh[handle], start, offset);

    return;
}

//printing to file stream

//print word/half word to file stream
static void fatout(ASMSys *system){
    //handle, integer
    system->cpu.status = system->cpu.curVal = 0;

    int handle = (int)(*system->cpu.arg[0]);
    if(_fatCheckHandle(system, handle)) return;
    else if(_fatCheckOpened(system, handle)){
#ifdef USE_64BIT
        system->cpu.curVal = fprintf (system->fh[handle], "%lld", (long long int)(*system->cpu.arg[1]));
#else
       system->cpu.curVal = fprintf (system->fh[handle], "%d", (*system->cpu.arg[1])); 
#endif
    }
        

    return;
}

//print a character to file stream
static void fatoutc(ASMSys *system){
    //handle, integer
    system->cpu.status = system->cpu.curVal = 0;

    int handle = (int)(*system->cpu.arg[0]);
    if(_fatCheckHandle(system, handle)) return;
    else if(_fatCheckOpened(system, handle))
        system->cpu.curVal = fprintf (system->fh[handle], "%c", (char)(*system->cpu.arg[1]));

    return;
}

static void fateof(ASMSys *system){
    //handle
    system->cpu.status = system->cpu.curVal = 0;

    int handle = (int)(*system->cpu.arg[0]);
    if(_fatCheckHandle(system, handle)) return;
    else if(_fatCheckOpened(system, handle))
        system->cpu.curVal = (!feof (system->fh[handle]));

    return;
}

static void fatremove(ASMSys *system){
    //path
    system->cpu.status = system->cpu.curVal = 0;

    //generate the file path
    char *path = (char*)system->cpu.arg[0];
    char newLine[MAX_PATH];
    //if an absolute path was given, use it
    BASM_CheckPathLocality(system, path, newLine, MAX_PATH);
    system->cpu.curVal = (!remove(newLine));
    return;
}

static void fatrename(ASMSys *system){
    //path
    system->cpu.status = system->cpu.curVal = 0;

    //generate the old file path
    char *oldPath = (char*)system->cpu.arg[0];
    char path1[MAX_PATH];
    BASM_CheckPathLocality(system, oldPath, path1, MAX_PATH);

    //generate the new file path
    char *newPath = (char*)system->cpu.arg[1];
    char path2[MAX_PATH];
    BASM_CheckPathLocality(system, newPath, path2, MAX_PATH);

    //rename the file
    system->cpu.curVal = (!rename(path1, path2));

    return;
}


#endif//BASM_COMPILE_FAT


/*==================================================================
    OP execution
==================================================================*/
int ASM_isLoopOp(ASMSys *system, bagOpInt op){
    if(system->global->op[op] == &loopto) return 1;
    else if(system->global->op[op] == &loopback) return 2;
    return 0;
}
//perform a cpu operation
void ASM_ExecOp(ASMSys *system){

    #if defined(BASM_COMPILE_VIDEO)
        system->video.update = 0;
    #endif
    
#ifdef USE_FUNC_ADDR
    void(*c)(ASMSys*)= (void *)system->cpu.curOp;
    if((bagDWord)c < (bagDWord)&jmpc && (bagDWord)c > (bagDWord)&jmpp) CPU_OP_INIT;
    c(system);
#else
    bagOpInt opNum = system->cpu.curOp;
    system->global->op[opNum](system);
#endif

    //set cpu flags
    if(system->cpu.curVal > 0)
        SET_FLAG(system->cpu.status, BIT_P);//positive bit
    else if(system->cpu.curVal < 0)
        SET_FLAG(system->cpu.status, BIT_N);//negative bit
    else if(system->cpu.curVal == 0){
        SET_FLAG(system->cpu.status, BIT_Z);//zero bit set
        //system->cpu.curVal++;
    }
    
    //if strict register usage is enabled
    if(GET_FLAG(system->flags, ASM_FORCEREGS)){
        //prevent overwriting of register 0 which is always 0
        system->cpu.reg[0] = 0;
        //add other stuff here
    }

    #if defined(COUNT_OPERATIONS)
        ++system->settings.operations;
    #endif
    return;
}


static void _setStaticOps(ASMSys *system, const char *label, int opNum, void *function){
    setCpuOps(system->global, label, opNum);
    system->global->op[opNum] = function;
    system->global->opArgCount[opNum] = 0;
}


void ASM_SystemYield(ASMSys *system) {
    yield(system);
}

void ASM_InitSTDOps(ASMSys *system){
    system->global->stdOpInit++;
    if(system->global->stdOpInit > 1) return;//only initialize ops once

    system->global->opLabels = NULL;
    system->global->opCount = 0;

    //set up some important ops first
    _setStaticOps(system, "HALT", HALT, (void*)&halt);
    _setStaticOps(system, "YIELD", YIELD, (void*)&yield);
    _setStaticOps(system, "NOP", NOP, (void*)&nop);
    _setStaticOps(system, "DATA", VAR, NULL);
    _setStaticOps(system, "ARRAY", ARRAY, NULL);


    //system, name, min number of parameters, function
    ASM_setCpuOpsEx(system->global, "GETARGV", 2, &getARGV);

    #if defined(PLATFORM_DS2)
        ASM_setCpuOpsEx(system->global, "SETCPUFREQ", 1, &setCPUFreq);
        ASM_setCpuOpsEx(system->global, "GETCPUFREQ", 1, &getCPUFreq);
    #endif

    //jumps
    ASM_setCpuOpsEx(system->global, "CALL", 5, &call);
    ASM_setCpuOpsEx(system->global, "JUMP", 1, &jump);
    ASM_setCpuOpsEx(system->global, "JUMPR", 1, &jumpr);
    ASM_setCpuOpsEx(system->global, "JMPN", 1, &jmpn);
    ASM_setCpuOpsEx(system->global, "JMPP", 1, &jmpp);
    ASM_setCpuOpsEx(system->global, "IFERR", 1, &jmpx);
    ASM_setCpuOpsEx(system->global, "JMPBK",0, &jmpbk);
    ASM_setCpuOpsEx(system->global, "JMPC", 1, &jmpc);
    ASM_setCpuOpsEx(system->global, "IFEQ", 3, &eq);
    ASM_setCpuOpsEx(system->global, "IFGT", 3, &gt);
    ASM_setCpuOpsEx(system->global, "IFGTE", 3, &gte);
    ASM_setCpuOpsEx(system->global, "IFLT", 3, &lt);
    ASM_setCpuOpsEx(system->global, "IFLTE", 3, &lte);
    
    //math
    ASM_setCpuOpsEx(system->global, "ABS", 2, &_abs);
    ASM_setCpuOpsEx(system->global, "ADD", 3, &add);
    ASM_setCpuOpsEx(system->global, "ADDU", 3, &addu);
    ASM_setCpuOpsEx(system->global, "SUB", 3, &sub);
    ASM_setCpuOpsEx(system->global, "SUBU", 3, &subu);
    ASM_setCpuOpsEx(system->global, "MUL", 3, &mul);
    ASM_setCpuOpsEx(system->global, "MULU", 3, &mulu);
    ASM_setCpuOpsEx(system->global, "DIV", 3, &_div);
    ASM_setCpuOpsEx(system->global, "DIVU", 3, &_divu);
    ASM_setCpuOpsEx(system->global, "REM", 3, &rem);
    ASM_setCpuOpsEx(system->global, "INCR", 1, &incr);
    ASM_setCpuOpsEx(system->global, "DECR", 1, &decr);
    ASM_setCpuOpsEx(system->global, "RANDM", 2, &randm);
    ASM_setCpuOpsEx(system->global, "SHL", 3, &shl);
    ASM_setCpuOpsEx(system->global, "SHLU", 3, &shlu);
    ASM_setCpuOpsEx(system->global, "SHR", 3, &shr);
    ASM_setCpuOpsEx(system->global, "SHRU", 3, &shru);
    ASM_setCpuOpsEx(system->global, "AND", 3, &and);
    ASM_setCpuOpsEx(system->global, "OR", 3, &or);
    ASM_setCpuOpsEx(system->global, "XOR", 3, &xor);
    ASM_setCpuOpsEx(system->global, "COMPL", 2, &compl);
    
    //output
    ASM_setCpuOpsEx(system->global, "OUT", 1, &out);
    ASM_setCpuOpsEx(system->global, "OUTU", 1, &outu);
    ASM_setCpuOpsEx(system->global, "OUTH", 1, &out);
    ASM_setCpuOpsEx(system->global, "OUTHU", 1, &outu);
    ASM_setCpuOpsEx(system->global, "OUTC", 1, &outc);
    //outcu
    ASM_setCpuOpsEx(system->global, "PRINT", 1, &print);
    ASM_setCpuOpsEx(system->global, "CLRS", 0, &clearConsole);
    
    //memory
    ASM_setCpuOpsEx(system->global, "CLR", 1, &clear);
    ASM_setCpuOpsEx(system->global, "CLRH", 1, &clear);
    ASM_setCpuOpsEx(system->global, "CLRC", 1, &clear);
    ASM_setCpuOpsEx(system->global, "LOAD", 2, &load);
    ASM_setCpuOpsEx(system->global, "LOADH", 2, &loadh);
    ASM_setCpuOpsEx(system->global, "LOADC", 2, &loadc);
    ASM_setCpuOpsEx(system->global, "STORE", 2, &store);
    ASM_setCpuOpsEx(system->global, "STOREH", 2, &storeh);
    ASM_setCpuOpsEx(system->global, "STOREC", 2, &storec);
    ASM_setCpuOpsEx(system->global, "SET", 2, &set);
    ASM_setCpuOpsEx(system->global, "GETARY2D", 4, &getary2d);
    ASM_setCpuOpsEx(system->global, "GETARY3D", 6, &getary3d);
    ASM_setCpuOpsEx(system->global, "LDADDR", 2, &laddr);
    ASM_setCpuOpsEx(system->global, "LAARY", 3, &laary);
    ASM_setCpuOpsEx(system->global, "LAARYH", 3, &laaryh);
    ASM_setCpuOpsEx(system->global, "LAARYC", 3, &laaryc);
    
    #ifdef BASM_ENABLE_STACK
        ASM_setCpuOpsEx(system->global, "PUSH", 1, &push);
        ASM_setCpuOpsEx(system->global, "PUSHT", 0, &pusht);
        ASM_setCpuOpsEx(system->global, "PUSHS", 0, &pushs);
    
        ASM_setCpuOpsEx(system->global, "POP", 1, &pop);
        ASM_setCpuOpsEx(system->global, "POPT", 0, &popt);
        ASM_setCpuOpsEx(system->global, "POPS", 0, &pops);
    #endif

    #if defined(ENABlE_TIME)
        ASM_setCpuOpsEx(system->global, "GETTIME", 1, &gettime);
    #endif


    #if defined(BASM_ALLOW_LOOPS)
        ASM_setCpuOpsEx(system->global, "LOOPTO", 4, &loopto);
        ASM_setCpuOpsEx(system->global, "LOOPBACK", 1, &loopback);
        ASM_setCpuOpsEx(system->global, "LOOPBREAK", 1, &loopbreak);
    #endif

    #if defined(BASM_ALLOW_STRINGS)
        ASM_setCpuOpsEx(system->global, "STRNCPY", 3, &_strncpy_);
        ASM_setCpuOpsEx(system->global, "STRNCAT", 3, &_strncat_);
        ASM_setCpuOpsEx(system->global, "STRLEN", 2, &_strlen_);
        ASM_setCpuOpsEx(system->global, "STRNCMP", 3, &_strncmp_);
        ASM_setCpuOpsEx(system->global, "ATOI", 2, &_atoi_);
    #endif

    //fat access
    #if defined(BASM_COMPILE_FAT)
        ASM_setCpuOpsEx(system->global, "FATOPEN", 3, &fatopen);
        ASM_setCpuOpsEx(system->global, "FATREAD", 4, &fatread);
        ASM_setCpuOpsEx(system->global, "FATWRITE", 4, &fatwrite);
        ASM_setCpuOpsEx(system->global, "FATCLOSE", 1, &fatclose);
        ASM_setCpuOpsEx(system->global, "FATFLUSH", 1, &fatflush);
        ASM_setCpuOpsEx(system->global, "FATSEEK", 3, &fatseek);
        ASM_setCpuOpsEx(system->global, "FATGETS", 3, &fatgets);
        ASM_setCpuOpsEx(system->global, "FATOUT", 2, &fatout);
        ASM_setCpuOpsEx(system->global, "FATOUTH", 2, &fatout);
        ASM_setCpuOpsEx(system->global, "FATOUTC", 2, &fatoutc);
        ASM_setCpuOpsEx(system->global, "FATEOF", 1, &fateof);
        ASM_setCpuOpsEx(system->global, "FATREWIND", 1, &fatrewind);
        ASM_setCpuOpsEx(system->global, "FATRM", 1, &fatremove);
        ASM_setCpuOpsEx(system->global, "FATRENAME", 2, &fatrename);
        ASM_setCpuOpsEx(system->global, "FATPRINT", 2, &fatprint);

    #endif

    //set depracated instructions
    ASM_setCpuOpsEx(system->global, "LDARY", 3, &ldary);
    ASM_setCpuOpsEx(system->global, "LDARYC", 3, &ldaryc);
    ASM_setCpuOpsEx(system->global, "LDARYH", 3, &ldaryh);
    ASM_setCpuOpsEx(system->global, "STARY", 3, &stary);
    ASM_setCpuOpsEx(system->global, "STARYC", 3, &staryc);
    ASM_setCpuOpsEx(system->global, "STARYH", 3, &staryh);
    
    SetDeprecated("STARY");
    SetDeprecated("STARYH");
    SetDeprecated("STARYC");
    SetDeprecated("LDARY");
    SetDeprecated("LDARYH");
    SetDeprecated("LDARYC");
    return;
}
