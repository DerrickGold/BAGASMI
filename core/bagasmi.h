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


#ifndef _ASM_SYSTEM_
#define _ASM_SYSTEM_

#ifdef __cplusplus
extern "C" {
#endif

//#define USE_FUNC_ADDR 1
//version number for compiled basm programs
#define BAGASM_VER 8

#include CONFIG_FILE
    
//=============================
//Variable types
//=============================

//used to identify which system the basm binary is compiled for
typedef enum {
    ASM_PLATFORM_DS2 = 1,
    ASM_PLATFORM_PC,
    ASM_PLATFORM_DS,
    ASM_PLATFORM_GBA,
    ASM_PLATFORM_OTHER=99,
}
ASM_System_Values;

typedef enum {
    BASM_EXIT_CLEAN = 1,
    BASM_EXIT_ERROR,
    BASM_EXIT_SUSPEND,
}BASM_Exit_Values;


#if defined(USE_64BIT)
    typedef int64_t bagDWord;
    typedef uint64_t bagUDWord;
#else
    typedef int32_t bagDWord;
    typedef uint32_t bagUDWord;
#endif
    
//typedef int64_t bagDWord;
//typedef uint64_t bagUDWord;
typedef int32_t bagWord;
typedef uint32_t bagUWord;    
typedef int16_t bagShort;
typedef uint16_t bagUShort;
typedef int8_t bagByte;
typedef uint8_t bagUByte;

//ram memory type
typedef bagUByte bagMemInt;
//pointer types
typedef bagUDWord bagAddrPtr;

//register type
typedef bagDWord bagReg;
typedef bagShort bagOpInt;

//=============================
//Memory alignment macros
//=============================

#ifndef BASM_NO_ALIGN_MEM
    #ifndef BASM_ALIGN_MEM
        #define BASM_ALIGN_MEM
    #endif
#endif

#ifdef BASM_ALIGN_MEM
    #define BASM_MEM_ALIGN_SIZE (sizeof(bagAddrPtr))

    #define BASM_MEM_ALIGN(ptr) \
        while(ptr%BASM_MEM_ALIGN_SIZE)ptr++;
#else
    #define BASM_MEM_ALIGN(ptr) ptr = ptr;
#endif

//=============================
//Special sizes to keep track of
//=============================
//instruction sizes
//16 bits for operation
//16 bits for which arguments to dereference
//32 bits free (if on 64 bit system, otherwise, full 32 bits taken)
//32/64 bits for each argument (addresses)    
#ifndef BASM_OPNUM_SIZE
    #define BASM_OPNUM_SIZE sizeof(bagOpInt)
#endif
    
#ifdef BASM_DEREF_SIZE
    #define BASM_DEREF_SIZE sizeof(bagOpInt)
#endif
    
#ifndef BASM_INSTRUCT_SIZE
    #define BASM_INSTRUCT_SIZE sizeof(bagUDWord)
#endif

    //size of each argument
#define BASM_ARG_SIZE sizeof(bagAddrPtr)

//=============================
//Memory reading for little/big endian
//=============================
//memory accessors for little endian
#if defined(CPU_LITTLE_ENDIAN)
    //read values from bytes
    #define READ_8BIT(mem) ((bagAddrPtr)(*mem))
    #define READ_16BIT(mem) \
        *(int16_t*)mem
    //#define READ_16BIT(mem) ((bagAddrPtr)(*(mem+1) + ((*(mem)) <<8)))
//    #define READ_32BIT(mem) ((bagAddrPtr)((*(mem)) + ((*(mem+1)) << 8) + ((*(mem+2)) << 16) + ((*(mem+3)) << 24)))
    #define READ_32BIT(mem) \
        *(int32_t*)mem
    
    
    
    #if defined(USE_64BIT)
        #define READ_64BIT(mem) \
            (*(uint64_t*)mem)
    
    #endif

    #define WRITE_8BIT(mem, val) ((*mem) = (bagMemInt)val)

    #define WRITE_16BIT(mem, val) \
        memcpy(mem, &(val), sizeof(int16_t));

   /* #define WRITE_16BIT(mem, val) {\
            bagUShort newVal = (bagUShort)val; \
            bagMemInt *p = (bagMemInt*)&newVal; \
            mem[0] = p[0]; \
            mem[1] = p[1];  \
            }
*/
    /*#define WRITE_32BIT(mem, val) {\
        bagUWord newVal = val; \
        bagMemInt *p = (bagMemInt*)&newVal; \
        mem[0] = p[0]; \
        mem[1] = p[1]; \
        mem[2] = p[2]; \
        mem[3] = p[3]; \
        }
     */
    #define WRITE_32BIT(mem, val) \
        memcpy(mem, &(val), sizeof(int32_t));


    #if defined(USE_64BIT)
        #define WRITE_64BIT(mem, val) \
            memcpy(mem, &(val), sizeof(int64_t));


    #endif
#elif defined(CPU_BIG_ENDIAN)
    //read values from bytes
   /* #define READ_8BIT(mem) ((bagWord)(*mem))
    #define READ_16BIT(mem) ((bagWord)((*(mem)) + ((*mem+1)<<8)))
    #define READ_32BIT(mem) ((bagWord)((*(mem+3)) + ((*(mem+2)) << 8) + ((*(mem+1)) << 16) + ((*(mem)) << 24)))

    #define WRITE_8BIT(mem, val) ((*mem) = (bagMemInt)val)


    #define WRITE_16BIT(mem, val) {\
            unsigned short newVal = (unsigned short)val; \
            bagMemInt *p = (bagMemInt*)&newVal; \
            (*mem) = p[1];mem++; \
            (*mem) = p[0];  \
        }

    #define WRITE_32BIT(mem, val) {\
        unsigned int newVal = val; \
        bagMemInt *p = (bagMemInt*)&newVal; \
        (*mem) = p[3];mem++; \
        (*mem) = p[2];mem++; \
        (*mem) = p[1];mem++; \
        (*mem) = p[0]; \
    }*/
#endif

//=============================
//Some rewiring stuff
//=============================
#ifndef BASM_PRINT
    #define BASM_PRINT printf
#endif

#ifndef BASM_DBGPRINT
    #define BASM_DBGPRINT printf
#endif

//inlined atoi function
#define BASM_atoi(s) atoi(s)
//much faster strcmp define
#define BASM_strcmp(s, t) ((s)[0] != (t)[0] ? (s)[0] - (t)[0] : \
        strcmp((s), (t)))

#ifndef BASM_DEFAULT_CONSOLE
    #define BASM_DEFAULT_CONSOLE 1
#endif

//=============================
//Parsing Symbols
//=============================
//configurable parsing symbol definitions from the config.h
#ifndef PARSE_HARDWARE
    #define PARSE_HARDWARE '$'
#endif

#ifndef PARSE_ADDRESS
    #define PARSE_ADDRESS '@'
#endif

#ifndef PARSE_STRING
    #define PARSE_STRING '"'
#endif

#ifndef PARSE_WORD
    #define PARSE_WORD '#'
#endif

#ifndef PARSE_SHORT
    #define PARSE_SHORT '*'
#endif

#ifndef PARSE_BYTE
    #define PARSE_BYTE '\''
#endif

#ifndef PARSE_ARGDIVIDER
    #define PARSE_ARGDIVIDER ','
#endif

#ifndef PARSE_COMMENT
    #define PARSE_COMMENT ';'
#endif

#ifndef PARSE_CHARACTER
    #define PARSE_CHARACTER '<'
#endif

#ifndef PARSE_LINE_ESCAPE
    #define PARSE_LINE_ESCAPE ' '
#endif

#ifndef PARSE_ARRAY_OFFSET
    #define PARSE_ARRAY_OFFSET '['
#endif

#ifndef PARSE_ARRAY_OFFSET_END
    #define PARSE_ARRAY_OFFSET_END ']'
#endif

#ifndef PARSE_DEREF_START
    #define PARSE_DEREF_START '('
#endif

#ifndef PARSE_DEREF_END
    #define PARSE_DEREF_END ')'
#endif

#ifndef PARSE_LABEL_END
    #define PARSE_LABEL_END ':'
#endif


//=============================
//Register labels
//=============================
//hardware definitions
//registers (0 to BASM_REGCOUNT)
#ifndef REGISTER_LABEL
    #define REGISTER_LABEL "R"
#endif

//Time to split up the registers

//zero register
#ifndef REGISTER_ZERO_LABEL
    #define REGISTER_ZERO_LABEL "zero"
#endif

#ifndef REGISTER_0_LABEL
    #define REGISTER_0_LABEL "0"
#endif

//temporary registers (t0, t1, t2, t3, t4, t5, t6, t7, t8, t9)
#ifndef REGISTER_TEMP_LABEL
    #define REGISTER_TEMP_LABEL "T"
#endif

//saved registers (s0, s1, s2, s3, s4, s5, s6, s7, s8, s9)
//extra s register to make up for lack of at register
#ifndef REGISTER_SAVED_LABEL
    #define REGISTER_SAVED_LABEL "S"
#endif

//function result registers (v0, v1)
#ifndef REGISTER_FUNCRES_LABEL
    #define REGISTER_FUNCRES_LABEL "V"
#endif

//function argument registers (a0, a1, a2, a3)
#ifndef REGISTER_FUNCARG_LABEL
    #define REGISTER_FUNCARG_LABEL "A"
#endif

//return address
#ifndef REGISTER_RETURN_LABEL
    #define REGISTER_RETURN_LABEL "RA"
#endif

//stack pointer (sp)
#ifndef REGISTER_STACKPTR_LABEL
    #define REGISTER_STACKPTR_LABEL "SP"
#endif

#ifndef REGISTER_RETURNADDR_LABEL
    #define REGISTER_RETURNADDR_LABEL "RA"
#endif
    
#ifndef REGISTER_MEMIO_LABEL
    #define REGISTER_MEMIO_LABEL "IO"
#endif

typedef enum {
    REG_ZERO = 0,
    
    //Temporary registers 1 - 10
    REG_T0, REG_T1, REG_T2, REG_T3, REG_T4,
    REG_T5, REG_T6, REG_T7, REG_T8, REG_T9,
    
    //Saved registers 11 - 21
    REG_S0, REG_S1, REG_S2, REG_S3, REG_S4,
    REG_S5, REG_S6, REG_S7, REG_S8, REG_S9,
    
    //argument registers 22-25
    REG_A0, REG_A1, REG_A2, REG_A3,
    
    //function return 26, 27
    REG_V0, REG_V1,
    
    
    //stack pointer
    REG_SP = 29,
    //return address
    REG_RA = 31,
    

    //input registers
    STYX_REG = 32,
    STYY_REG = 33,
    PAD_REG = 34,
    REG_MEMIO = 35,
    
    BASM_REGCOUNT,
}REGISTER_MAPPING;


//=============================
//Extra hardware registers
//todo: set up extra hardware through memory mapped io system
//=============================

//sprite x y locations
//these will be deprecated soon( can use 4 arg functions now)
//bg x and y scrolling
#ifndef BACKGROUND_X
    #define BACKGROUND_X "BGX"
#endif

#ifndef BACKGROUND_Y
    #define BACKGROUND_Y "BGY"
#endif


//stylus/mouse access
#ifndef STYLUS_X
    #define STYLUS_X "PENX"
#endif

#ifndef STYLUS_Y
    #define STYLUS_Y "PENY"
#endif


#ifndef LCDSCREEN_BUF
    #define LCDSCREEN_BUF "LCD"
#endif

//argv accessor
#ifndef ARG_V
    #define ARG_V "ARGV"
#endif

//Get program root directory
#ifndef INFO_LABEL
#define INFO_LABEL "_INFO"
#define INFO_ROOT "_INFO_ROOTDIR"
#endif

//=============================
/*
 * System hardware
 *
*/
//=============================

#ifndef BASM_MAX_OPS
    #define BASM_MAX_OPS 512
#endif

//4 kilobytes of memory reserved for default memory size
#ifndef BASM_MEMSIZE
    #define BASM_MEMSIZE (1024*sizeof(bagUDWord))
#endif

//set up memory to use for stack (default 64 words)
#ifndef BASM_STACKSIZE
    #define BASM_STACKSIZE ( 2048 * sizeof(bagUDWord))
#endif

#ifdef BASM_MEMMAPPED_IO
    #ifndef BASM_MEMIO_SIZE
        #define BASM_MEMIO_SIZE 128
    #endif
#endif
    
//label length for operation names, variables, jumps
#ifndef BASM_LABELLEN
    #define BASM_LABELLEN 128
#endif
//length for data input for variables
#ifndef BASM_ARGLEN
    #define BASM_ARGLEN 64
#endif
//max length for strings
#ifndef BASM_STRLEN
    #define BASM_STRLEN 256
#endif
//maximum arguments to scan for in a string
#ifndef BASM_MAX_ARGS
    #define BASM_MAX_ARGS 12
#endif

#if defined(BASM_ALLOW_LOOPS)
    #ifndef BASM_LOOP_DEPTHS
        #define BASM_LOOP_DEPTHS 8
    #endif
#endif

#ifndef BASM_JUMPBACK_COUNT
    #define BASM_JUMPBACK_COUNT 128
#endif

//number of file handles per system instance
#ifndef BASM_FILEHANDLE_COUNT
    #define BASM_FILEHANDLE_COUNT 2
#endif

/*
 * All operations organized alphabetically for binary search of operations
*/
//These CPU operations will have a consistant operation number
//that is referenced in the interpreter core. The order of these
//operations are very important and must remain static.
typedef enum{
    HALT, YIELD, NOP, VAR, ARRAY,
}IMPORTANT_CPU_OPS;



//cpu flags
typedef enum{
    BIT_P = (1<<0),
    BIT_Z = (1<<1),
    BIT_N = (1<<2),
    BIT_X = (1<<3),
    BIT_COLLISION = (1<<4),
    BIT_HALT = (1<<6),
    BIT_YIELD = (1<<7),
}ASM_CPUFlags_e;


typedef struct ASM_ARGV{
    char **argv;
    bagWord argc;
    bagByte allocated;
    bagMemInt *tempram;
}ASM_ARGV;


typedef struct RingBuf{
    bagWord start, end, total, cur;
}RingBuf;

//loaded .asm file information

//stores a single line from a file
typedef struct ASMLineInfo{
    bagWord len;
    bagUByte args;
    bagByte *str;
}ASMLineInfo;

//a file full of lines
typedef struct ASMFile{
    ASMLineInfo *line;
    bagDWord line_count;
    bagByte _skipCurLine; //skip current line when reading in file
}ASMFile;

//stores program positions, jumpback positions and a file of lines
typedef struct ASMProgram{
    bagDWord pos, preJumpPos[BASM_JUMPBACK_COUNT]; //lines;//position in program mem for reading
    RingBuf jmpBackPos;

    ASMFile file;
}ASMProgram;

//used for temporarely storing and organizing jump positions
//and variable declarations
typedef struct ASMData{
    char *label;
    //char label[BASM_LABELLEN];
    bagAddrPtr addr;
    //char size;
}ASMData;

//program pragmas set on launch
typedef struct ASMSettings{
    //default pragmas
    bagWord debugTxt, runTime, fatLog, dumpMem, passive;

    //ds2 specific pragmas
    #if defined(PLATFORM_DS2)
        bagWord flipMode;
    #endif

    //enable text console, how much ram to allocate for the program
    bagWord console, totalRam;

    //number of operations executed
    #ifdef COUNT_OPERATIONS
        bagUWord operations;
    #endif

    //screen size and position
    bagWord screenWidth, screenHeight, x, y;

    //max number of args expected for argv
    bagWord argCount;
}ASMSettings;


/*===================================================================================
Hardware stuff
===================================================================================*/

#if defined(BASM_ALLOW_LOOPS)
    typedef struct ASMLoop{
        //store a start and end position for the loop
        bagUDWord pos[BASM_LOOP_DEPTHS << 1];
        //store to and increment values
        bagAddrPtr *counter[BASM_LOOP_DEPTHS];
        bagAddrPtr *to[BASM_LOOP_DEPTHS];
        bagAddrPtr *increment[BASM_LOOP_DEPTHS];
    }ASMLoop;
#endif


//system virtual cpu stuff
typedef struct ASMCpu{
    //cpu registers
    bagReg reg[BASM_REGCOUNT];
    //current calculated value
    volatile bagDWord curVal;
    //cpu bit flag status
    bagUByte status;
    //current operation
    bagOpInt curOp;
    //operation arguments
    bagAddrPtr *arg[BASM_MAX_ARGS];
    //exit code and frequency (DS2 cpu speed)
    bagWord exit, freq;
}ASMCpu;


//systems virtual memory
typedef struct ASMMemory{
    bagUDWord pos;//end position of used memory
    bagAddrPtr *posReg;//register for ram end position

    #if defined(BASM_USE_STACK_MEM)
        bagMemInt byte[BASM_MEMSIZE];
    #else
        bagMemInt *byte;//ram
    #endif
    
    //stack memory
#ifdef BASM_ENABLE_STACK
    bagMemInt stack[BASM_STACKSIZE];
#endif
    
#ifdef BASM_MEMMAPPED_IO
    bagMemInt *memio[BASM_MEMIO_SIZE];
#endif

} ASMMemory;



//platform support some sort of graphics output?
#if defined(BASM_COMPILE_VIDEO)
    #ifndef BASM_MAX_SPRITES
        #define BASM_MAX_SPRITES 16
    #endif

    #ifndef BASM_MAX_BACKGROUNDS
        #define BASM_MAX_BACKGROUNDS 4
    #endif

    #ifndef BASM_MAX_FONTS
        #define BASM_MAX_FONTS 4
    #endif
    #ifndef BASM_MAX_TEXTBOXS
        #define BASM_MAX_TEXTBOXS 4
    #endif


    typedef struct ASMVideo{
        VIDEO_STRUCT
        //whether any of the screens are updated
        bagWord update;
    }ASMVideo;


#endif


//platform support input?
#if defined(BASM_COMPILE_INPUT)
    typedef struct ASMInput{
        bagAddrPtr *x, *y;//stylus/mouse x and y position
        bagAddrPtr *pad;//button input
    }ASMInput;
#endif





struct ASMSys;
//Operations will be shared among all systems loaded
typedef struct ASMGlobal_s{
    //storing of operations
    ASMData *opLabels;
    bagWord opCount;
    void (*op[BASM_MAX_OPS])(struct ASMSys *);
    bagUByte opArgCount[BASM_MAX_OPS];

    //checks if operations have been initiated
    bagByte stdOpInit, vidOpInit, inputOpInit, extLibInit;
}ASMGlobal;
static ASMGlobal ASM_Globals;


//the main system struct
typedef struct ASMSys{
    bagWord flags;
    ASM_ARGV passed_args;
    //program settings
    ASMSettings settings;

    //should point to ASM_Globals
    ASMGlobal *global;

    //system cpu
    ASMCpu cpu;

    //jump declarations
    bagWord jumpCount;
    ASMData *jumps;

    //variable declarations
    bagWord dataCount;
    ASMData *data;


    //system ram
    ASMMemory memory;

    //video core
    #if defined(BASM_COMPILE_VIDEO)
        ASMVideo video;
    #endif
    //input core
    #if defined(BASM_COMPILE_INPUT)
        ASMInput input;
    #endif

    #if defined(BASM_ALLOW_LOOPS)
        ASMLoop loops;
    #endif

    //program information
    ASMProgram prgm;

    #if defined(BASM_COMPILE_FAT)
        FILE *fh[BASM_FILEHANDLE_COUNT];
    #endif

    #if defined(BASM_COMPILE_EXTLIBS)
    EXTLIBS_DATA
    #endif

    //current directory of launched asm file
    FilePath rootPath;

    #if defined(OUTPUT_LINES_FILE)
        FilePath prgmPath;
    #endif

    bagByte cleaned;//if the system has been cleaned
    bagUWord memUsed;//total memory the system is using itself.
}ASMSys;


//settings for the interpretter
typedef enum{
    ASM_DATAPURGE = (1 << 0),
    ASM_JUMPPURGE = (1 << 1),
    ASM_COMPILE = (1 << 2),
    ASM_EXPORTOPS = (1<<3),
    ASM_DUMPLINES = (1<<4),
    ASM_AUTOERROR = (1<<5),
    ASM_LINEBYLINE = (1<<6),
    ASM_FORCEREGS = (1<<7),
}ASM_initFlags_e;
    
#if defined(USE_64BIT)
typedef enum {
    BASMC_ADDR_RAM = (1<<30),
    BASMC_ADDR_REG = (1<<31),
} BASMCompiled_ADDR;
#else
typedef enum {
    BASMC_ADDR_RAM = (1<<30),
    BASMC_ADDR_REG = (1<<31),
} BASMCompiled_ADDR;
#endif


typedef struct ASMHeader{//header for compiled code file
    bagByte magicNumber[8];//.BASM
    //file indentification
    bagByte platform, //platform compiled for
            version; //version of program compiled with

    //system components
    bagUDWord ram, //start position of ram
              ramSize, //size of ram used when program starts
              totalRam,//total amount of ram allocated for system
              prgm; //position in file where code starts
    bagByte lenSize;//number of bits used to indicate line length before a line

    //program specifics
    bagByte flip, //flip mode to update screen
            console, //if the console is enabled or disabled
            script;//if system should use resources passively

    bagUShort screenWd,//screen
              screenHt;//screen height
    bagWord lines;//number of lines  in file
    bagUByte argCount;//number of args
}ASMHeader;

//=================================
//Publically accessable functions
//=================================

//Initialize launch arguments for the virtual system
extern void ASM_SetArgs(ASMSys *system, int argc, ...);
extern int ASM_SetArgsV(ASMSys *system, int argc, va_list *vl);
extern void ASM_InitARGV(ASMSys *system, int argc, char **argv);

//Initialize the System
extern void ASM_ResetSys(ASMSys *system);
extern char ASM_InitSystem(const char *file, ASMSys *system, int flags);

//execute a line of code loaded in the system
extern int ASM_Step(ASMSys *system);

//loads a file, and compiles it to binary data
extern int ASM_BuildFile(ASMSys *system, const char *file);

//Stops and clears all data used by the interpreter
extern void ASM_CleanSystem(ASMSys *system);

//execute a line of code given by "input"
int ASM_ExecuteLine(ASMSys *system, char *input, int *pushLines, int *pushDepth);


//=================================
//Functions used among the codebase
//=================================
//a quick function for initializing, loading, and running a file
//exits when the asm program ends
extern int ASM_run(ASMSys *system, const char *file);

//used for initializing new operations
extern void ASM_setCpuOpsEx(ASMGlobal *global, const char *name, char params, void (*function)(ASMSys *system));

//stuff that needs to be accessed from other system files
extern volatile bagUWord ASM_MEMUSED;

//error and debug stuff
extern void errorMsg(ASMSys *system, bagDWord line, const char *msg, ...);
extern void printFunction(ASMSys *system, const char *text, ...);

//localize a path to the interpreters loaded binary directory
//extern void AddRootPath(const ASMSys *system, const char *filepath, char *outPath, int outLen);
void BASM_CheckPathLocality(ASMSys *system, const char *inPath, char *newPath, size_t newLen);

//Set a funciton to deprecated status
extern void SetDeprecated(const char *name);

#ifdef __cplusplus
}
#endif


#endif
