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

/*===================================================================================
*   Video functions if available
*
*
*
===================================================================================*/
#if defined(BASM_COMPILE_VIDEO)
    #include "video.h"
#endif

/*===================================================================================
*   input functions if available
*
*
*
===================================================================================*/
#if defined(BASM_COMPILE_INPUT)
    #include "input.h"
#endif


/*===================================================================================
*   Any external libraries for the platform
*
*
*
===================================================================================*/
#if defined(BASM_COMPILE_EXTLIBS)
    #include "extLibs.h"
#endif





//#define DEBUG_OUTPUT_LISTS
/*===================================================================================
*   System stuff
*
*
*
===================================================================================*/
volatile bagUWord ASM_MEMUSED = 0;
//buffers to relieve stack space
//these are kind of a mess, definitely need to be renamed/moved/fixed at some point or other
static char ASM_LineBuf[BASM_STRLEN];

//name of platform running on
extern const char Platform[256];


/*===================================================================================
*   Misc functions
*
*
*
===================================================================================*/


extern void waitExit(void);

#if defined(RUNTIME)
    extern void Timer_start();
    extern float Timer_getTicks();
#endif

//Import from ASM_ExecOps.c
extern void ASM_InitSTDOps(ASMSys *system);
extern void ASM_ExecOp(ASMSys *system);
extern void ringBuf_init(RingBuf *buffers, int bufCount);


//adds the current root path to the specified file path
static void _AddRootPath(ASMSys *system, const char *filepath, char *outPath, size_t outLen){
    int oldDepth = system->rootPath.depth;
    if(FilePath_Len(&system->rootPath) < outLen){
        FilePath_Convert(&system->rootPath, filepath, (int)strlen(filepath));
        //strncpy(outPath, system->curRoot, outLen);
        FilePath_Export(&system->rootPath, outPath, (int)outLen);//export a relative path
        //clean up added paths back to root
        while(system->rootPath.depth > oldDepth){
            FilePath_Remove(&system->rootPath);
        }
    }
    return;
}

void BASM_CheckPathLocality(ASMSys *system, const char *inPath, char *newPath, size_t newLen){
    if(inPath[0] == BASM_DIR_SEPARATOR){
        strncpy(newPath, inPath, MAX_PATH);
    } else {
        //otherwise its a local path
        _AddRootPath(system, inPath, newPath, newLen);
    }
}

//Get the directory path to the launched file
static int _getFilePath(const char *path, char *pathbuf, size_t outSize){
    char localPath[4] = {'.', BASM_DIR_SEPARATOR, '\0'};
    
    char *dirSep = strrchr(path, BASM_DIR_SEPARATOR);
    if(!dirSep) {
        strncpy(pathbuf, localPath, strlen(localPath));
    } else {
        size_t copyLen = (size_t)(dirSep - path);
        if(copyLen > 1) strncpy(pathbuf, path, copyLen);
        else strncpy(pathbuf, localPath, strlen(localPath));
        
    }
    return 1;
}


static long stringToNumber(const char *string) {
    //first check if number is in hex
    if (!strncasecmp(string, "0x", strlen("0x"))) {
        return strtol(string, NULL, 16);
    }
    //then check for binary
    else if (!strncasecmp(string, "0b", strlen("0b"))) {
        return strtol(string + strlen("0b"), NULL, 2);
    }
    //finally, just assume its base ten
    return strtol(string, NULL, 10);
}




//Set operations or registers as deprecated
#define MAX_DEPRECATED 10
static char deprecatedList[MAX_DEPRECATED][BASM_LABELLEN];
static int deprecatedCount = 0;

void SetDeprecated(const char *name){
    if(deprecatedCount >= 5)
        return;
    strncpy(deprecatedList[deprecatedCount], name, BASM_LABELLEN);
    deprecatedCount++;
    return;
}


static void check_Deprecated(ASMSys *system, const char *name){
    for(int i = 0; i < deprecatedCount; i++){
        size_t length = strlen(deprecatedList[i]);
        

        //if(deprecatedList[i][length] == '*' || deprecatedList[i][length-1] == '*')
        //    length--;
        //else
        //    length = BASM_LABELLEN;

        if(!strncasecmp(name, deprecatedList[i], length)){
            printFunction(system, "%s is deprecated.\n", name);
            break;
        }
    }
    return;
}

/*===================================================================================
*   Debug printing
*
*
*
===================================================================================*/
//static char __dbg_print_buf[512];
void printFunction(ASMSys *system, const char *text, ...){
    if(!system) {
        printf("missing system!\n");
        return;
    }
    //print nothing if
    if(!system->settings.console || !text)
        return;

//TODO: find varargs workaround for issue in 64bits
    char __dbg_print_buf[MAX_PATH];
    va_list vl;
    va_start(vl, text);
    vsnprintf(__dbg_print_buf, MAX_PATH, text, vl);
    va_end(vl);

    switch(system->settings.passive){
        case 1:
            BASM_PRINT("%s", __dbg_print_buf);
        break;
        default:
            BASM_PRINT("%s", __dbg_print_buf);
        break;
    }
    return;
}

void PrintBanner(ASMSys *system, const char *platformName){
    printFunction(system, "\n=============================\n");
    printFunction(system, "BAGASMI %s Interpreter RC%d\n", platformName, BAGASM_VER);
    printFunction(system, "=============================\n\n");
    return;
}
//For error messages that output the line where an error occurs
#if defined(OUTPUT_LINES_FILE)

static void lineDumpPath(const char *root, char *outBuf, size_t outLen){
    char endStr[] = "_lines.txt";
    size_t endLen = strlen(endStr);
    char linesName[MAX_PATH];

    strncpy(linesName, root, MAX_PATH);
    char *pos = strrchr(linesName, '.');
    if(!pos){
        if(strlen(linesName) + endLen < MAX_PATH)
            strncat(linesName, endStr, endLen);
    } else {
        if((int)(pos - linesName) + endLen < MAX_PATH)
            strncpy(pos, endStr, endLen);
    }
    strncpy(outBuf, linesName, outLen);
    return;
}

static void removeLineDump(const char *path){
    char linesName[MAX_PATH];
    lineDumpPath(path, linesName, MAX_PATH);

    struct stat st;
    if(lstat(linesName, &st)>= 0)
        remove(linesName);
    return;
}

static FILE *openLineDump(const char *path, const char *mode){
    char linesName[MAX_PATH];
    lineDumpPath(path, linesName, MAX_PATH);
    return fopen(linesName, mode);
}

static char *_getLineDump(FILE *file, bagDWord lineNum){
    if(!file) return NULL;

    bagUWord curLine = 0;

    while(!feof(file)){
        //read line
        if(!fgets(ASM_LineBuf, BASM_STRLEN, file)) {
            goto ERROR;
        }
        else if(curLine == lineNum) {
            //strip off newline
            char *end = strrchr(ASM_LineBuf, '\n');
            if(end) *end = '\0';
            break;
        }

        curLine++;
    }

    ERROR:
    fclose(file);
    return ASM_LineBuf;
}

#endif

void errorMsg(ASMSys *system, bagDWord lineNum, const char *msg, ...){
    //do nothing if an error has already been reported
    if(system->cpu.exit == BASM_EXIT_ERROR) return;

    va_list vl;
    va_start(vl, msg);

    char dbgOut[MAX_PATH];
    vsnprintf(dbgOut, MAX_PATH, msg, vl);
    va_end(vl);

    if(lineNum > -1){
#ifdef USE_64BIT
        BASM_DBGPRINT("[BAGASMI DBG MSG] @ line: %lld\n%s ", (long long int)lineNum, dbgOut);
#else
        BASM_DBGPRINT("[BAGASMI DBG MSG] @ line: %d\n%s ", lineNum, dbgOut);
#endif
    }
    else {
        BASM_DBGPRINT("[BAGASMI DBG MSG]\n%s", dbgOut);
    }

    //if interpreting line by line, we can warn users of errors so they can fix
    //without stopping the script
    if( !GET_FLAG(system->flags, ASM_LINEBYLINE)){

	    //print out the line which caused the problem
	    #if defined(OUTPUT_LINES_FILE)
	        if(GET_FLAG(system->flags, ASM_DUMPLINES) && lineNum > -1) {
	            char tempPath[MAX_PATH];
	            if(!FilePath_Export(&system->prgmPath, tempPath, MAX_PATH)){
	                BASM_DBGPRINT("Error creating program path\n");
	            } else {
	                FILE *lines = openLineDump(tempPath, "rb");
	                sprintf(dbgOut, "First use in line:\n\"%s\"\n\n", _getLineDump(lines, lineNum));
	                BASM_DBGPRINT("%s", dbgOut);
	            }
	        }
	    #endif

	    #ifndef BASM_NO_FORCED_EXIT
	        waitExit();
	    #endif

	    ASM_CleanSystem(system);
	}

	system->cpu.exit = BASM_EXIT_ERROR;
    return;
}


/*===================================================================================
*   Memory allocators
*
*
*
===================================================================================*/

#define asm_calloc(ptr, count, size){ \
    ASM_MEMUSED += (count * size); \
    ptr = calloc(count, size); \
}
//printf("calloc: %d\n", ASM_MEMUSED); \


#define asm_free(ptr, count, size) {\
    ASM_MEMUSED -= (count*size);\
    if(ASM_MEMUSED < 0) ASM_MEMUSED = 0;\
    free(ptr); \
    ptr = NULL; \
}
//printf("free: %d\n", ASM_MEMUSED); \

#define asm_realloc(temp, ptr, size, oldSize){ \
    ASM_MEMUSED -= oldSize; \
    ASM_MEMUSED += size; \
    temp = realloc(ptr, size); \
}
//printf("realloc: %d\n", ASM_MEMUSED); \







//clear and free all memory used by data storage
static ASMData *allocateMoreData(ASMData *data, bagWord *count){

    if(*count <= 0){
        *count = 0;
        data = NULL;
    }

    //we need to always allocate one more than necessary
    bagWord oldSize = ((*count)) * sizeof(ASMData);(*count)++;
    bagWord newSize = ((*count)) * sizeof(ASMData);

    ASMData *tempdat = NULL;
    asm_realloc(tempdat, data, newSize, oldSize);
    if(!tempdat)
        return NULL;

    return tempdat;
}

static void cleanAllData(ASMData *data, bagWord *count){\
    ASMData *lines = (ASMData *)(data);
    bagWord total = (*count);

    //make sure there is data first
    if(!lines || total == 0)
        return;

    //clear all label names
    for(int i = 0; i < total; i++){
        if(lines[i].label){
            asm_free(lines[i].label, sizeof(char), strlen((const char*)lines[i].label));
        }

        lines[i].label = NULL;
        lines[i].addr = 0;
    }

    asm_free(lines, (*count), sizeof(ASMData));
    *count = 0;
    return;
}


/*===================================================================================
 *   Setting and clearing lines in an asm program
 *
 *
 *
 ===================================================================================*/

void ASMLine_Clear(ASMLineInfo *curLine) {
    if(curLine->str && curLine->len >= 0){
        asm_free(curLine->str, curLine->len, sizeof(char));
    }
    curLine->str = NULL;
    curLine->len = 0;
    return;
}

int ASMLine_Set(ASMLineInfo *curLine, const char *newData, int size){
    size_t len = 0;
    ASMLine_Clear(curLine);
    if(size < 0) len = strlen(newData);
    else len = abs(size);
    len++;
    
    asm_calloc(curLine->str, len + 1, sizeof(bagByte));
    if(!curLine->str)
        return -1;
    
    if(size > 0)
        memcpy(curLine->str, newData, len * sizeof(char));
    
    else if(size < 0){
        strncpy((char*)curLine->str, newData, len);
    }
    curLine->len = (bagWord)len;
    
    return 0;
}


/*===================================================================================
*   Data sorting/retrieving for jumps, variable declarations, and cpu operations
*
*   Uses qsort and binary search algorithms
*
===================================================================================*/
#define DATA_END_OFFSET 1

static inline void swap_internal(bagByte *a, bagByte *b, size_t size){
    if (a == b)
        return;

    bagByte t;
    while (size--){
        t = *a;
        *a++ = *b;
        *b++ = t;
    }
    return;
}

static void qsort_internal(bagByte *begin, bagByte *end, size_t size, int(*compar)(const void *, const void *)){
    if (end <= begin) return;

    bagByte *pivot = begin;
    bagByte *l = begin + size, *r = end;
    

    while (l < r){
        int result = compar(l, pivot);

        if (result > 0){
            l += size;
            __builtin_prefetch(l);
        }
        else if(result < 0){
            r -= size;
            __builtin_prefetch(r);
            swap_internal(l, r, size);
        }
        else
            break;
    }

    l -= size;
    swap_internal(begin, l, size);
    qsort_internal(begin, l, size, compar);
    qsort_internal(r, end, size, compar);
    return;
}

static void _BAG_Qsort(void *data, size_t dataCount,
                        size_t dataSize, int(*compar)(const void *, const void *)){

    qsort_internal((bagByte *)data, data+dataCount*dataSize, dataSize, compar);
    return;
}


static int compareSort(const void *dest, const void *src){
    ASMData *destObj = (ASMData*)dest;
    ASMData *srcObj = (ASMData*)src;

    char *dest_str = (char*)destObj->label,
         *src_str = (char*)srcObj->label;

    return -BASM_strcmp(dest_str, src_str);
}

//case insensitive compare
static int compareSortNoCase(const void *dest, const void *src){
    ASMData *destObj = (ASMData*)dest;
    ASMData *srcObj = (ASMData*)src;
    
    char *dest_str = (char*)destObj->label,
    *src_str = (char*)srcObj->label;
    
    return -strcasecmp(dest_str, src_str);
}

//storing and retrieving variables declared
static void asm_listSort(ASMData *data, bagWord count){
    _BAG_Qsort((void*)data, count, sizeof(ASMData), &compareSort);

#ifdef DEBUG_OUTPUT_LISTS
    printf("\nSorted List\n");
    for(int i = 0; i < count; i++)
        printf("%s\n", data[i].label);
#endif
    return;
}

static void asm_listSortNoCase(ASMData *data, bagWord count){
    _BAG_Qsort((void*)data, count, sizeof(ASMData), &compareSortNoCase);
}

static bagWord _BAG_binSearch(const void *data,  bagWord dataCount, bagWord dataSize, void *searchVal, int(*compare)(const void *, const void*)){
    bagWord min = 0, max = dataCount - 1,
             mid = 0, cmp = 0;

    if(max < 0)
        return -2;

    do{
        mid = (min + max) >> 1;
        cmp = compare((void*)data + (mid * dataSize), searchVal);

        if(cmp == 0)
            return mid;
        else if(cmp < 0)
            min = mid + 1;
        else if(cmp > 0)
            max = mid - 1;

    }while(min <= max);

    return -1;
}

static int compareFind(const void *data, const void *cmpVal){
    ASMData *destObj = (ASMData*)data;

    char *dest_str = (char*)destObj->label,
         *src_str = (char*)cmpVal;

    //printf("%s vs. %s\n", dest_str, src_str);
    return BASM_strcmp(dest_str, src_str);
}

static int compareFindNoCase(const void *data, const void *cmpVal){
    ASMData *destObj = (ASMData*)data;
    
    char *dest_str = (char*)destObj->label,
    *src_str = (char*)cmpVal;
    
    //printf("%s vs. %s\n", dest_str, src_str);
    return strcasecmp(dest_str, src_str);
}

static bagWord asm_listGet(const ASMData *data, bagWord count, const char *label){
    return _BAG_binSearch((void*)data,  count,
                            sizeof(ASMData), (char*)label, &compareFind);
}

//Operations are not case sensitive anymore
static bagWord asm_listGetNoCase(const ASMData *data, bagWord count, const char *label){
    return _BAG_binSearch((void*)data,  count,
                          sizeof(ASMData), (char*)label, &compareFindNoCase);
}

/*===================================================================================
*   Address reading and writing for BAGASMI
*
*
*
===================================================================================*/
//for reading and storing hardware locations when parsing / optimizing file
static inline void storeAddress(bagAddrPtr value, bagUByte *buf){
    switch(sizeof(bagAddrPtr)){
        #if defined(USE_64BIT)
            case 8:
            WRITE_64BIT(buf, value);
            break;
        #endif
        case 4:WRITE_32BIT(buf, value);break;
        case 2:WRITE_16BIT(buf, value);break;
        case 1:WRITE_8BIT(buf, value);break;
    }
    return;
}
bagAddrPtr readAddress(bagUByte *buf){
    switch(sizeof(bagAddrPtr)){
        #if defined(USE_64BIT)
            case 8: return READ_64BIT(buf); break;
        #endif
        case 4: return READ_32BIT(buf);break;
        case 2: return READ_16BIT(buf); break;
        case 1: return READ_8BIT(buf); break;
    }
}

//convert variables and hardware consts to their memory addresses
//then store addresses in the virtual ram
static int setAddr(bagAddrPtr *addr, bagByte *output){
    bagAddrPtr temp = (bagAddrPtr)addr;
    storeAddress(temp, (bagUByte*)output);
    output += sizeof(bagAddrPtr);
    (*output) = '\0';
    return BASM_ARG_SIZE;
}

/*===================================================================================
*   CPU Section
*   Configuring of cpu operations
*
*
===================================================================================*/

void setCpuOps(ASMGlobal *global, const char *name, bagDWord opVal){
    //old is system->opLabels
    ASMData *temp = allocateMoreData(global->opLabels, &global->opCount);
    if(!temp){
        //errorMsg(system, -1, "Error allocating op labels");
        printf("error initializing ops\n");
        return;
    }
    global->opLabels = temp;

    bagWord count = (global->opCount - DATA_END_OFFSET);
    if(count < 0){
        printf("cpu ops negative\n");
        count = 0;
    }
    //increase memory used
    size_t len = strlen(name);
    //if(len > BASM_LABELLEN) len = BASM_LABELLEN - 1;
    asm_calloc(global->opLabels[count].label, sizeof(char), len + 1);
    if(!global->opLabels[count].label){
        printf("error initializing ops: 2\n");
        return;
    }
    memcpy(global->opLabels[count].label, name, len);
    global->opLabels[count].label[len] = '\0';
    //strncpy(global->opLabels[count].label, name, len);
    global->opLabels[count].addr = opVal;
    //printf("Op %s added.\n", system->opLabels[count].label);
    return;
}

void ASM_setCpuOpsEx(ASMGlobal *global, const char *name, char params, void (*function)(ASMSys *system)){
    bagWord opNumber = global->opCount;
    //printf("opnum:%d -> name: %s\n", opNumber, name);
    setCpuOps(global, name, opNumber);
    global->op[opNumber] = (void*)function;
    global->opArgCount[opNumber] = params;
    return;
}

static void ASM_cleanCpuOps(ASMGlobal *global){
    global->stdOpInit--;

    //if no more standard ops have been initialized, then assume no more systems are running
    if(global->stdOpInit <= 0){
        cleanAllData((ASMData*)global->opLabels, &global->opCount);
        global->vidOpInit = 0;
        global->inputOpInit = 0;
        global->stdOpInit = 0;
        global->extLibInit = 0;
    }
    return;
}

static void asmParseGetOp(ASMSys *system, ASMLineInfo *line){
    bagByte *curLine = line->str;

    //get the next system operation from code and collect arguments to execute
#ifdef USE_FUNC_ADDR
    system->cpu.curOp = readAddress((bagUByte*)curLine);
#else
    system->cpu.curOp = *(bagOpInt*)(curLine);
#endif
    curLine += BASM_INSTRUCT_SIZE;
    //get the bits indicating which arguments need to be dereferenced
    bagOpInt deref = *(bagOpInt*)((line->str + BASM_OPNUM_SIZE));
    register int i = 0;
    //set input to the end of its buffer to use for checking in the loop
    while(i < line->args){
        //the operation will pick which arguments it needs
        //memory address (most likely pointing to hardware), these are all fixed lengths
        system->cpu.arg[i] = (deref & (1 << (i ))) ?
            (bagAddrPtr*)*((bagAddrPtr*) readAddress((bagMemInt*)curLine)) : (bagAddrPtr*) readAddress((bagMemInt*)curLine);

        i++;
        //check for next argument now
        curLine += BASM_ARG_SIZE;
    }
    system->cpu.arg[i] = NULL;

    #if defined(DBGTEXT)
        if(system->settings.debugTxt)
            printFunction(system, "Got Op: %d, args: %d\n",
                            system->cpu.curOp, line->args);
    #endif

    ++system->prgm.pos;
    return;
}

int ASM_StepEx(ASMSys *system, ASMLineInfo *line){
    //fetch new command
    asmParseGetOp(system, line);
    //execute command
    ASM_ExecOp(system);
    //return if program ends
    return !system->cpu.exit;
}

//retrieve and parse the next line in a program
inline int ASM_Step(ASMSys *system){
	return ASM_StepEx(system, &system->prgm.file.line[system->prgm.pos]);
}

/*===================================================================================
*   Memory Section
*
*
*
===================================================================================*/
static bagWord asmGetRamSize(ASMSys *system){
    return system->settings.totalRam;
}

static void asmSetRamSize(ASMSys *system, bagWord size){
    system->settings.totalRam = size;
    return;
}

bagAddrPtr *asmGetRamAddr(ASMSys *system, long pos){
    return (bagAddrPtr*)&system->memory.byte[pos];
    //return (bagAddrPtr*)system->memory.posReg + pos;
}

//initialize the systems ram
static int asmInitRam(ASMSys *system){
    if(asmGetRamSize(system) == 0)
        asmSetRamSize(system, BASM_MEMSIZE);

    #ifndef BASM_USE_STACK_MEM
        //if we aren't using stack memory, allocate some memory to use
        asm_calloc(system->memory.byte, asmGetRamSize(system), sizeof(bagMemInt));
        if(!system->memory.byte) return 0;
    #else
        //memset(system->memory.byte, 0, system->settings.totalRam);
    #endif

    system->memory.pos = (system->settings.argCount*sizeof(bagAddrPtr));
    system->memory.posReg = (bagAddrPtr*)system->memory.byte;
    //(bagAddrPtr*)&system->memory.pos;
    
    return 1;
}

//free allocated memory space if not using stack memory
static void asmCleanRam(ASMSys *system){
    #ifndef BASM_USE_STACK_MEM
        if(system->memory.byte){
            asm_free(system->memory.byte, asmGetRamSize(system), sizeof(bagMemInt));
        }
        system->memory.byte = NULL;
    #endif
    return;
}

//memory operations

//byte sized data (1 byte)
static int _checkMemLimit(ASMSys *system, int size){
    if(system->memory.pos + size > asmGetRamSize(system)){
        errorMsg(system, -1, "Out of memory!\n");
        return 0;
    }
    return 1;
}

static int asm_storeByte(ASMSys *system, bagDWord value, bagAddrPtr addr){
    if(addr > system->memory.pos &&
       !_checkMemLimit(system, sizeof(bagDWord))) return 0;

    WRITE_8BIT(asmGetRamAddr(system, addr), (bagByte)value);
    //update memory position if we are writing past it
    if(addr >= system->memory.pos)
        system->memory.pos += sizeof(bagByte);
    return 1;
}


//half word sized data (2 bytes)
static int asm_storeShort(ASMSys *system, bagDWord value, bagAddrPtr addr){
    if(addr > system->memory.pos &&
       !_checkMemLimit(system, sizeof(bagUWord))) return 0;

    bagAddrPtr *temp = asmGetRamAddr(system, addr);
    WRITE_16BIT(temp, value);
    //writing a 16bit value will take up the current address, and the next
    //address in memory
    addr += (sizeof(bagShort) - 1);

    //update memory position if we are writing past it
    if(addr > system->memory.pos)
        system->memory.pos += sizeof(bagShort);

    return 1;
}


//word sized data(4 bytes in 32 bit, 8 bytes in 64 bits)
static int asm_storeWord(ASMSys *system, bagDWord value, bagAddrPtr addr){
    if(addr > system->memory.pos &&
       !_checkMemLimit(system, sizeof(bagDWord))) return 0;

    bagAddrPtr *temp = asmGetRamAddr(system, addr);
    #if defined(USE_64BIT)
        WRITE_64BIT(temp, value);
    #else
        WRITE_32BIT(temp, value);
    #endif
    //writing a 32bit value will take up the current address,
    //and the next 3 address spaces in memory.
    addr += (sizeof(bagDWord));

    //update memory position if we are writing past it
    if(addr > system->memory.pos)
        system->memory.pos += sizeof(bagDWord);

    return 1;
}

//store data for later use when substituting text for addresses
static bagDWord asm_dataSet(ASMSys *system, const char *label, bagDWord value,
                            int (*store)(ASMSys *, bagDWord, bagAddrPtr)){

    //check if a store function was given
    if(!store){
        printFunction(system, "Null store method given\n");
        return system->memory.pos;
    }

    //otherwise...
    //store label name and get position in memory to refer to
    //in the data struct.
    bagWord curData = system->dataCount - DATA_END_OFFSET;

    if(curData < 0)
        curData = 0;

    size_t len = strlen(label);
    //if(label[len-1] == PARSE_LABEL_END){
    //    len--;
   // }
    char *end = strrchr(label, PARSE_LABEL_END);
    if(end) len--;
    //copy name and address (allocate one extra byte for null)
    asm_calloc(system->data[curData].label, sizeof(bagByte), len + 1);
    if(!system->data[curData].label){
        errorMsg(system, -1, "Error allocating data label\n");
        return system->memory.pos;
    }

    memcpy(system->data[curData].label, label, len);
    system->data[curData].label[len] = '\0';
    system->data[curData].addr = system->memory.pos;

    //store the value to the systems memory
    if(!(*store)(system, (bagUWord)value, (int)system->memory.pos)) return -1;

    //return next free memory position
    return system->memory.pos;
}


/*===================================================================================
*   Scripts ARGV Protocol
*   Passing string arguments to the script
*
*
===================================================================================*/
//Pushes args from temp ram to main ram, to be called once main ram is initialized
static int asm_pushArgs(ASMSys *system){
    //if args are stored in temporary ram, copy them to main ram
    if(system->passed_args.argc > 0 && system->passed_args.tempram &&
        system->passed_args.tempram != system->memory.byte){
        memcpy(asmGetRamAddr(system, 0), system->passed_args.tempram, system->memory.pos);

        asm_free(system->passed_args.tempram, system->passed_args.argc, sizeof(bagAddrPtr));
        system->passed_args.tempram = NULL;
    }
    return 1;
}

//clean up memory used for allocating ARGV
void ASM_ClearArgs(ASMSys *system){
    if(system->passed_args.argc <= 0 || !system->passed_args.argv)
        return;

    for(int i = 0; i < system->passed_args.argc; i++){
        char *str = system->passed_args.argv[i];
        if(str && system->passed_args.allocated){
            asm_free(system->passed_args.argv[i], strlen((char*)str), sizeof(char));
        }

        system->passed_args.argv[i] = NULL;
    }

    if(system->passed_args.allocated){
        asm_free(system->passed_args.argv, sizeof(char*), system->passed_args.argc);
    }

    system->passed_args.argv = NULL;
    system->passed_args.argc = 0;
    system->passed_args.allocated = 0;
    return;
}

//initialize ARGV for a system using char **
void ASM_InitARGV(ASMSys *system, int argc, char *argv[]){
    system->passed_args.tempram = NULL;
    system->passed_args.argv = NULL;
    system->passed_args.argc = 0;

    if(argc > 0 && argv){
        system->passed_args.argv = argv;
        system->passed_args.argc = argc;

        system->passed_args.tempram = system->memory.byte;
        if(!system->passed_args.tempram)//if main ram isn't available, make temporary ram
            asm_calloc(system->passed_args.tempram, argc, sizeof(bagAddrPtr));

        if(!system->passed_args.tempram){
            errorMsg(system, -1, "Error allocating arg ram\n");
            return;
        }
        bagMemInt *mem = system->passed_args.tempram;
        for(int i = 0; i < argc; i++){
            storeAddress((bagAddrPtr)argv[i], (bagMemInt*)mem);
            mem += sizeof(bagAddrPtr);
        }
   }
   return;
}

//set up ARGV for a system by passing a va_list
int ASM_SetArgsV(ASMSys *system, int argc, va_list *vl){
    if (argc <= 0) return 0;

    ASM_ClearArgs(system);
    char **argv = NULL;
    asm_calloc(argv, argc, sizeof(char*));
    if(!argv){
        printFunction(system,"Error allocating args\n");
        return -1;
    }
    for(int i = 0; i < argc; i++){
        char *str = va_arg(*vl, char *);
        size_t len = strlen(str);
        asm_calloc(argv[i], len + 1, sizeof(char));
        if(!argv[i]){
            errorMsg(system, -1, "Error allocating args\n");
            break;
        }
        else strncpy(argv[i], str, len);
    }
    system->passed_args.allocated = 1;
    ASM_InitARGV(system, argc, argv);
    return 1;
}

//set up ARGV for a system
void ASM_SetArgs(ASMSys *system, int argc, ...){
    if (argc <= 0) return;

    va_list vl;
    va_start(vl, argc);
    ASM_SetArgsV(system, argc, &vl);
    va_end(vl);
    return;
}





/*===================================================================================
*   File Parsing and optimization
*
*
*
===================================================================================*/

/*
*Everything after this point focuses on converting the source file to
*a more optimized format to obtain faster parsing speeds from the above
*functions.
*/


/*
The following section focuses on converting data to its memory
address locations for fast reading
*/


/*
This will scan an argument at a time, with a string taking up a whole argument
and assumes an arg divider is placed immediately after the end string marker
*/
static int parseInLine(bagByte *line, bagByte *out, char escapeChar){
    register int len = 0, quoteCount = 0;
    //start string
    while((*line) != escapeChar && (*line) >= ' '){
        /*
        *check if the line contains a string
        *if there is a string, the function won't exit
        *until the string ends and it finds the escape character
        */
        if(!quoteCount && (*line) == PARSE_STRING){
            quoteCount++;
            escapeChar = PARSE_STRING;
        }
        len++;//keep track of how much string we went through
        if(!quoteCount && *line == ' ') line++;
        else (*out++) = (*line++);

        //make sure that people can print out the escape character here if PARSE_CHARACTER operator
        //is in use
        bagByte *temp = line - 1;
        if(*temp == PARSE_CHARACTER) (*out++) = (*line++);

    }
    //take into account end quote for argument size
    (*out) = '\0';
    if(quoteCount > 0 && len <= 1) len = 0;
    //detect the end of the string
    else if(quoteCount >= 1 && *line == PARSE_STRING) len++;

    return len;
}

//Check if a given variable arguement contains an array offset ->[ ]
static bagWord checkIsArray(const char *label){
    /*char tempLabel[MAX_PATH];
    strncpy(tempLabel, label, MAX_PATH);
    char *result = strchr(tempLabel, PARSE_ARRAY_OFFSET);
    if(result) return (bagWord)(result - tempLabel);
    else return (bagWord)strlen(label);*/
    char *result = strchr(label, PARSE_ARRAY_OFFSET);
    if(result) return (bagWord)(result - label);
    
    return (bagWord)strlen(label);
}

//Parse the array offset of a given variable
static bagWord parseArrayOffset(char *input){
    char tempLabel[MAX_PATH];
    strncpy(tempLabel, input, MAX_PATH);

    char *start = strchr(tempLabel, PARSE_ARRAY_OFFSET);
    char *end = strchr(tempLabel, PARSE_ARRAY_OFFSET_END);
    if(start && end){
        start++;
        char posBuf[32];
        char *ptr = &posBuf[0];
        while(start < end){
            *ptr = *start;
            start++;
            ptr++;
        }
        *ptr = '\0';
        int offset = atoi(posBuf);
        return offset;
    }
    return 0;
}


/*===================================================================================
*   Convert labels into addresses for various systems
*
===================================================================================*/

//set the addresses for misc requested info
bagAddrPtr *ASM_setInfoAddr(ASMSys *system, char *label){
    bagAddrPtr *data = NULL;

    if(!strncmp(label, INFO_ROOT, strlen(INFO_ROOT))){
        //label += strlen(INFO_ROOT);

        FilePath_Export(&system->rootPath, ASM_LineBuf, BASM_STRLEN);
        data = (bagAddrPtr*)&ASM_LineBuf[0];
    }

    return data;
}

static int setRegAddrEx(ASMSys *system, int regNum, bagByte *output, bagByte deref){
    if(regNum == REG_SP) {
        bagAddrPtr **sp = (bagAddrPtr**)&system->cpu.reg[regNum];
        setAddr(*sp, output);
        
    }
    bagAddrPtr *ptr = (bagAddrPtr*)&system->cpu.reg[regNum];
    return setAddr(ptr, output);
}

//set the address for a register
static int setRegAddr(ASMSys *system, char *label, bagByte *output, bagByte deref){
    //regName, regNumber
    //zero,    0
    //t0-t9,   1-10
    //s0-s9,   11 - 21
    //a0-a3,   22,23,24,25
    //v0, v1,  26, 27
    //sp       29
    int regNum = 0;
    
    
    //make sure the labels that contain the most characters are checked first
    //so no false positives are found for labels that may contain similar characters
    //but less of them.
#if defined(BASM_ENABLE_STACK)
    if (!strncasecmp(label, REGISTER_STACKPTR_LABEL, strlen(REGISTER_STACKPTR_LABEL))) {
        regNum = REG_SP;
    } else
#endif
        
    if (!strncasecmp(label, REGISTER_RETURNADDR_LABEL, strlen(REGISTER_RETURNADDR_LABEL))){
        regNum = REG_RA;
    }
    else if (!strncasecmp(label, REGISTER_TEMP_LABEL, 1)) { //t registers
        int tempReg = BASM_atoi(++label);
        regNum = (tempReg % 10) + REG_T0;
    }
    else if (!strncasecmp(label, REGISTER_SAVED_LABEL, 1)) { //s registers
        int tempReg = BASM_atoi(++label);
        regNum = (tempReg % 10) + REG_S0;
    }
    else if(!strncasecmp(label, REGISTER_FUNCARG_LABEL, 1)){//a registers
        int tempReg = BASM_atoi(++label);
        regNum = (tempReg & 3) + REG_A0;
    }
    else if(!strncasecmp(label, REGISTER_FUNCRES_LABEL, 1)){//v registers
        int tempReg = BASM_atoi(++label);
        regNum = (tempReg & 1) + REG_V0;
    }
    else if (!strncasecmp(label, REGISTER_ZERO_LABEL, strlen(REGISTER_ZERO_LABEL)) ||
             !strncasecmp(label, REGISTER_0_LABEL, 1))
    {
        //regNum should already be 0
    }
    else if (!strncasecmp(label, REGISTER_LABEL, 1)) {
        if(GET_FLAG(system->flags, ASM_FORCEREGS))
            printFunction(system, "\n[Warning]: Strict Registers enabled, generic register usage detected, may overwrite in use registers!\n");
        regNum = BASM_atoi(++label);//original R registers
    }
    
    //no proper label found
    else return 0;
    
    
    //consume the PARSE_DEREF_END character
    return setRegAddrEx(system, regNum, output, deref);
}

//set address for constant values
//Constant values that are used more than once can be optimized so only
//a single instance of it remains in memory. Otherwise more ram is used for each constant.
//A constant can be a text string "Like This" or numbers.
static int setConstantAddr(ASMSys *system, const char *label, bagByte *output){

    #if defined(OPTIMIZE_CONSTANT_STORAGE)

        bagWord index = asm_listGet(system->data, system->dataCount, label);
        if(index < 0){
            ASMData *temp = allocateMoreData(system->data, &system->dataCount);
            if(!temp){
                errorMsg(system, -1, "Error allocating const value: \"%s\"\n", label);
                return 0;
            }
            system->data = temp;
            //this constant has yet to be stored;
            if(asm_dataSet(system, label, (bagDWord)stringToNumber(label), &asm_storeWord)<0)
                return 0;

            asm_listSort(system->data, system->dataCount);
            if((index = asm_listGet(system->data, system->dataCount, label)) < 0){
                errorMsg(system, -1, "Failed storing const: \"%s\"\n", label);
                return 0;
            }
        }

        bagAddrPtr *mem = (bagAddrPtr*)asmGetRamAddr(system, system->data[index].addr);
        return setAddr((bagAddrPtr*)mem, output);

    #else

        if(!asm_storeWord(system, (bagDWord)stringToNumber(label), system->memory.pos))
            return 0;

        bagAddrPtr *mem = (bagAddrPtr*)asmGetRamAddr(system, system->memory.pos - sizeof(bagAddrPtr));
        return setAddr((bagAddrPtr*)mem, output);
    #endif
}

//Store a string constant such as "Text in quotes!"
static int setStringConstantAddr(ASMSys *system, char *label, bagByte *output, size_t length){
    if(system->memory.pos + length > asmGetRamSize(system)){
        errorMsg(system, -1, "String exceeds memory space:\n\"%s\"\n", label);
        return 0;
    }

    //fix string formatting, allow \n stuff
    char *pos = strchr(label, '\\');
    while(pos){
        size_t curLen = (size_t)(pos - label);
        char fixLine = 0;
        if(pos[1] == 'n'){
            *pos = '\n';
            fixLine = 1;
        } else if(pos[1] == 't'){
            *pos = '\t';
            fixLine = 1;
        }

        if(fixLine){
            if(length - 1  > curLen){
                memcpy(&pos[1], &pos[2], sizeof(char) * (length - curLen));
            } else {
                pos[1] = '\0';
            }
        }

        //find next character
        pos = strchr(pos, '\\');
    }


    bagAddrPtr *mem = (bagAddrPtr*)asmGetRamAddr(system, system->memory.pos);
    memcpy(&mem[0], label, sizeof(char) * length-1);
    mem[length] = '\0';
    system->memory.pos += length;
    //make sure every string is stored in a size that is a power of 4
    BASM_MEM_ALIGN(system->memory.pos);
    return setAddr(mem, output);
}

//store address for variables that have been declared
static int setVariableAddr(ASMSys *system, char *label, bagByte *output, bagDWord lineNum){
    char tempName[BASM_LABELLEN];
    bagWord copyLen = checkIsArray(label);
    if(copyLen > BASM_LABELLEN) copyLen = BASM_LABELLEN-1;
    //printf("label:%s - copylen = %d - oldLen: %d\n", label, copyLen, strlen(label));
    memcpy(tempName, label, sizeof(char) * copyLen);
    tempName[copyLen] = '\0';

    bagWord index = asm_listGet(system->data, system->dataCount, tempName);
    if(index < 0){
        errorMsg(system, lineNum,"No such label declared: \"%s\" -line:%d\n", tempName, lineNum);
        return 0;
    } else if(system->data[index].addr >= asmGetRamSize(system)){
        errorMsg(system, lineNum,"Memory address exceeds memory range: \"0x%x\"\n",
                 system->data[index].addr);
        return 0;
    }
    bagAddrPtr *mem = (bagAddrPtr*)asmGetRamAddr(system, system->data[index].addr);

    if(strlen(label) > copyLen){
        label+=copyLen;
        bagWord offset = parseArrayOffset(label);
        if(system->data[index].addr +  offset >= asmGetRamSize(system) || offset < 0){
            errorMsg(system, lineNum, "Array index out of bounds: \"%s\"\n", label);
            return 0;
        }

        mem = (bagAddrPtr*)asmGetRamAddr(system, system->data[index].addr + offset);
    }
    return setAddr((bagAddrPtr*)mem, output);
}


//Store addresses of jump locations
static int setJumpAddr(ASMSys *system, char *label, bagByte *output, bagDWord lineNum){
    bagWord test = asm_listGet(system->jumps, system->jumpCount, label);
    if(test < 0) return 0; //jump doesn't exist

    //fast jumps convert the number to a packed integer on the virtual ram
    //this removes the need for an atoi call in the processing loop later on
    BASM_MEM_ALIGN(system->memory.pos);
    if(!asm_storeWord(system, system->jumps[test].addr, system->memory.pos))
        return -1;

    bagAddrPtr *mem = (bagAddrPtr*)asmGetRamAddr(system, system->memory.pos - sizeof(bagAddrPtr));
    BASM_MEM_ALIGN(system->memory.pos);
    //length till second argument from first(or in this case, the end of the line)
    return setAddr((bagAddrPtr*)mem, output);
}



/*===================================================================================
*   Master function that converts text to addresses
*
===================================================================================*/

static void characterToAscii(char *buffer){
    char tempBuf[6];//should only need 4 characters max

    char *buf = buffer;
    if(*buf != PARSE_CHARACTER)
        return;

    buf++;
    //if just a normal character, copy it over
    if(*buf == '\\'){
        //otherwise we need to combine some characters
        char *temp = buf + 1;
        switch(*temp){
            case 'n': *buf = '\n'; break;
            case 'b': *buf = '\b'; break;
            case 't': *buf = '\t'; break;
            case 'r': *buf = '\r'; break;
        }
    }

    sprintf(tempBuf, "%c%d", PARSE_BYTE, (int)*buf);
    strncpy(buffer, tempBuf, 6);
    return;
}


//Converts all arguments in a line to its proper addresses
static int varhwToAddr(ASMSys *system, ASMLineInfo *prgmLine, bagDWord lineNum, bagByte *outBuf, int outBufSize){

    bagByte *out = (outBuf + BASM_INSTRUCT_SIZE),
            *linePos = (prgmLine->str + BASM_INSTRUCT_SIZE);
    char labelBuf[BASM_STRLEN];
    int arg = 0, totalLen = 0, exitFlag = 0, finalLen = 0;
    bagOpInt argDeref = 0;

    for(arg = 0; arg < BASM_MAX_ARGS; arg++){

        //grab argument from line, argument label length is returned
        int argLen = parseInLine(linePos + totalLen, (bagByte*)labelBuf, PARSE_ARGDIVIDER);
        //printf("curArg: %s\n", labelBuf);
        if(argLen <= 0) break;

        //increase line position
        totalLen += (argLen + 1);
        if(totalLen == 0 || linePos[0] == '\0'){
            //exit if end of line is hit
            prgmLine->args = 0;
            break;//exit scanning
        }

        //otherwise, we have found an argument
        prgmLine->args++;
        if(abs(totalLen - (bagWord)strlen((char*)linePos)) <= 1){
            //check if this will be the last argument
            //if it is, we need to exit after processing it
            exitFlag++;
        }

        //if value is not a variable address then copy it to the line and check again
        check_Deprecated(system, (char*)labelBuf);

        //convert ascii characters to ascii numbers
        characterToAscii((char*)labelBuf);
        
        bagByte deref = 0;
        if(labelBuf[0] == PARSE_DEREF_START){
            deref++;
            argDeref |= (1<< (arg));
        }

        //make sure we have room in our output buffer
        if(finalLen > outBufSize){
            system->cpu.exit = BASM_EXIT_ERROR;
            return 0;
        }
        else if(labelBuf[deref] == PARSE_HARDWARE){
            //hardware location addresses
            bagByte *label = (bagByte*)&labelBuf[deref]; label++;
            finalLen += setRegAddr(system, (char*)label, &out[finalLen], deref);

            //bagAddrPtr *infoData = ASM_setInfoAddr(system, label);
            //if(infoData) finalLen += setAddr(infoData, &out[finalLen]);

            #if defined(BASM_COMPILE_VIDEO)
                bagAddrPtr *vidData = ASM_setVideoAddr(system, (char*)label);
                if(vidData) finalLen += setAddr(vidData, &out[finalLen]);
            #endif

            #if defined(BASM_COMPILE_INPUT)
                bagAddrPtr *inData = ASM_setStylusAddr(system, (char*)label);
                if(inData) finalLen += setAddr(inData, &out[finalLen]);
            #endif
        }
        //constant values can be converted to save an atoi call later
        else if(labelBuf[0] == PARSE_WORD || labelBuf[0] == PARSE_SHORT || labelBuf[0] == PARSE_BYTE)
            finalLen += setConstantAddr(system, &labelBuf[1], &out[finalLen]);

        //strings and jumps not converted
        else if(labelBuf[deref] == PARSE_STRING){
            finalLen += setStringConstantAddr(system, &labelBuf[1], &out[finalLen], argLen);
        }
        else{//convert variable to address
            //check for a number
            if(labelBuf[0] >= 48 && labelBuf[0] <= 57){
                finalLen += setConstantAddr(system, labelBuf, &out[finalLen]);
            } else {
                int test = setJumpAddr(system, labelBuf, &out[finalLen], lineNum);
                if(!test) finalLen += setVariableAddr(system, labelBuf, &out[finalLen], lineNum);
                else if(test > 0) finalLen += test;
            }
        }


        //exit on errors
        if(system->cpu.exit) return 0;
        if(exitFlag) break;//make sure the last argument has a length
    }

    //shrink the current line as needed
    int newLen = (int)(finalLen + BASM_INSTRUCT_SIZE);
    if(ASMLine_Set(prgmLine, (char*)outBuf, newLen)){
        errorMsg(system, lineNum, "Error compacting line: %d", lineNum);
        return 0;
    }

    //append dereferenced args to the op number
    bagShort *derefBits = (bagShort*)(prgmLine->str + BASM_OPNUM_SIZE);
    *derefBits = argDeref;
    //    (*(prgmLine->str + BASM_OPNUM_SIZE)) = argDeref;
    return 1;
}

/*
*   This function parses a line for a DATA declaration.
*   If a line has data declared on it (ie and integer),
*   it is stored on the virtual ram, and its position
*   and name are stored in a list to be used later.
*
*   This also detects
*/


//ToDo:Fix so that jump labels can exist on lines without trailing instruction
//If no trailing instruction exists, need to remove line
//store jump points in a list for use when compiling code on load
static void storeJumpPoint(ASMSys *system, const char *dataLabel, ASMLineInfo *currentLine, bagDWord lineNum){
    
    //resize jump list to accomadate the new jump
    ASMData *temp = allocateMoreData(system->jumps, &system->jumpCount);
    if(!temp){
        errorMsg(system, lineNum, "Error storing jump: \"%s\"\n", dataLabel);
        return;
    }
    system->jumps = temp;
    int count = system->jumpCount - DATA_END_OFFSET;
    if(count < 0){
        printf("jump count negative\n");
        count = 0;
    }


    
    size_t labelLen = strlen(dataLabel);
    //copy data to list
    char *end = strchr(dataLabel, PARSE_LABEL_END);
    if(end) labelLen--;
    
    
    //make sure to allocate extra space for null character
    asm_calloc(system->jumps[count].label, sizeof(char), labelLen + 1);
    if(!system->jumps[count].label){
        //printf("error initializing ops: 2\n");
        errorMsg(system, lineNum, "Error allocating jump label\n");
        return;
    }
    strncpy(system->jumps[count].label, dataLabel, sizeof(char) * labelLen);
    system->jumps[count].label[labelLen] = '\0';
    //printf("Stored: %s\n", system->jumps[count].label);
    system->jumps[count].addr = (bagWord)lineNum;//jump destination in the form of program lines

    if (!system->prgm.file._skipCurLine) {
        //if we aren't skipping this line, that is, this line contains a jump label AND
        //an instruction, then....
        //get rid of jump labels for faster parsing later (jump label + space)
        //strip any odd spaces between the label and operation
        size_t len = labelLen;
        while(currentLine->str[len] == PARSE_LABEL_END || currentLine->str[len] == ' ' || currentLine->str[len] == '\t') len++;
        //copy line excluding label and spaces
        char *start = (char*)&currentLine->str[len];
        size_t newLen = currentLine->len - len;
        strncpy(ASM_LineBuf, start, newLen);

        //resize the line
        if(ASMLine_Set(currentLine, ASM_LineBuf, (bagWord)newLen)){//negative size to force a strcpy
            errorMsg(system, lineNum, "Error allocating jump: \"%s\"\n", dataLabel);
            return;
        }
    }
    return;
}

//stores a data declaration to a list for further processing
static void storeDataDeclaration(ASMSys *system, const char *dataLabel, char *input,
                                ASMLineInfo *currentLine, bagDWord lineNum, int isArray){

    char type = *input++,
         multiplier = 0;

    int (*storeFun)(ASMSys *, bagDWord, bagAddrPtr) = NULL;

    
    //Check type then write the value of the variable to ram
    switch(type){
        default:
            errorMsg(system, lineNum, "Unsupported memory type: \"%c\"\n", type);
            return;
        break;
        case PARSE_WORD: storeFun = &asm_storeWord; multiplier = sizeof(bagUDWord); break;
        case PARSE_SHORT: storeFun = &asm_storeShort; multiplier = sizeof(bagShort); break;
        case PARSE_BYTE: storeFun = &asm_storeByte; multiplier = sizeof(bagByte); break;
    }
    
    //resize the variable data struct to accomadate the new declaration
    ASMData *temp = allocateMoreData(system->data, &system->dataCount);
    if(!temp){
        errorMsg(system, lineNum, "Error storing Variable: \"%s\"\n", dataLabel);
        return;
    }
    system->data = temp;

    //if this is a variable, this value is the default value
    //if this is an array, this value dictates how large the array is
    bagDWord value = stringToNumber(input);
    
    if(asm_dataSet(system, dataLabel, value, storeFun) < 0) return;
    
    
    //allocate more space for arrays
    if(isArray) {
        bagDWord arraySize = (multiplier * (value));
        if (system->memory.pos +  arraySize >= asmGetRamSize(system)){
            errorMsg(system, lineNum, "Array exceeds memory limit: %d\n", arraySize);
            return;
        }
        
        //Check if there are any array values that need to be stored
        size_t pos = strcspn ((char*)currentLine->str, input);
        
        if(pos > 0 && pos < currentLine->len) {
            //update pointer position to start with array default value contents
            char *arrayDefaults = (char*)currentLine->str + pos;
            
            //check for a default string
            char *temp = strchr(arrayDefaults, '\"');
            if (temp) {
                
                char *strEnd = strchr(++temp, '\"');
                if (!strEnd) goto arrayEnd;
                
                //get ram address to store content
                bagAddrPtr *ram = asmGetRamAddr(system, system->memory.pos);
                //clear ram and copy the string
                memset(ram, 0, arraySize);
                strncpy((char*)ram, temp, (strEnd - temp));
                
            }
            //otherwise, check for { .., ..., ...., } etc..
            else {
                
                temp = strchr(arrayDefaults, '{');
                if(!temp) goto arrayEnd;
                
                bagAddrPtr oldPos = system->memory.pos - multiplier;
                const char delims[] = { ' ', ',', '{', '}' };
                char *tok = strtok(temp, delims);
                while(tok && oldPos < system->memory.pos + arraySize) {
                    size_t val = stringToNumber(tok);
                    //memcpy(ram, &val, multiplier);
                    storeFun(system, val, oldPos);
                    oldPos += multiplier;
                    tok = strtok(NULL, delims);
                }
            }
        }
    arrayEnd:
        system->memory.pos += arraySize;
    }
    
    BASM_MEM_ALIGN(system->memory.pos);

    //change the line to NOP because we don't need to data in the program buffer anymore after this point
    if(ASMLine_Set(currentLine, "NOP\0", 4)){//negative size to force a strcpy
        errorMsg(system, lineNum, "Error allocating variable: \"%s\"\n", dataLabel);
        return;
    }

    currentLine->args = 0;
    return;
}


static size_t _splitLineData(ASMLineInfo *currentLine, int offset, char *output, int outSize){
    if(offset >= currentLine->len)
        return 0;

    size_t len = 0;
    const char tokens[4] = {PARSE_LABEL_END, ' ', '\t', '\0'};
    
    //get position of first tokenizing character in string
    char *pos = strpbrk((char*)currentLine->str + offset, tokens);
    if(!pos) {
        pos = strrchr((char*)currentLine->str, '\0');
        if(!pos) pos = (char*)&currentLine->str[currentLine->len];
    }
    //consume the label end as well, it gets stripped off later
    else if (*pos == PARSE_LABEL_END) pos++;
    
    //get length from beginning of string to the tokenized character
    len = (pos - (char*)currentLine->str) - offset;
    if(len > outSize) len = outSize - 1;

    //copy section of string to output
    //printf("copying size: %d\n", len);
    memcpy(output, currentLine->str + offset, sizeof(char) * len);
    if(len <= outSize) output[len] = '\0';

    while(*pos == ' ' || *pos == '\t'){
        pos++;
        len++;
    }
    return len;
}

//ToDo:Fix so that jump labels can exist on lines without trailing instruction
static int parseLineData(ASMSys *system, ASMLineInfo *currentLine, bagDWord lineNum){
    /*
    *if the line is a variable declaration, we need to collect the variable name,
    *then store its value in the virtual ram.
    */
    int _txtBufferSize = (BASM_LABELLEN<<1) + BASM_STRLEN;
    char _txtBuffer[_txtBufferSize];
    memset(_txtBuffer, 0, _txtBufferSize);
    
    char     *dataLabel = (char*)&_txtBuffer[0],
             *opLabel = (char*)&_txtBuffer[BASM_LABELLEN],
             *input = (char*)&_txtBuffer[BASM_LABELLEN<<1];
    
    //find data label
    int offset = 0;
    size_t test = _splitLineData(currentLine, 0, dataLabel, BASM_LABELLEN);
    if(!test) return -2;
    offset += test;
    
    //find op label
    test = _splitLineData(currentLine, offset, opLabel, BASM_LABELLEN);
    if(!test) {
        //in this case, we may just have a jump label on its own
        //so we should insert a NOP here.
        //first check that we do have a label
        if(strchr (dataLabel, PARSE_LABEL_END)){
            //line contains a label ending character, so put in a nop
            memcpy(opLabel, "NOP", (sizeof(char) * strlen("NOP")));
            ASMLine_Set(currentLine, opLabel, (bagWord)strlen(opLabel));
            //signal that this line needs to be overwritten by the next line during
            //the file reading process
            system->prgm.file._skipCurLine++;
        } else {
            //otherwise, return an error
            return -3;
        }
    } else {
        offset += test;
    }
    
    
    
#ifdef DEBUG_OUTPUT_LISTS
    printf("%s->%s\n", dataLabel, opLabel);
#endif

    if(!strlen(opLabel)) return 1;//no label for operation found
    //make sure opLabel actually contains an op code
    int tempOperation = asm_listGetNoCase(system->global->opLabels, system->global->opCount, opLabel);
    if(tempOperation < 0){//if not, then no label was found the first time
#ifdef DEBUG_OUTPUT_LISTS
        printf("%s no op\n", opLabel);
#endif
        return 1;
    }
    bagDWord operation = system->global->opLabels[tempOperation].addr;
    int storeArray = 0;
    switch(operation){
        case -1: break;
        default://jump labels are usually declared infront of normal operations
            storeJumpPoint(system, dataLabel, currentLine, lineNum);
        break;
        case ARRAY:
            storeArray++;
        case VAR://data declaration via the DATA operation
            //collect input value
            test = _splitLineData(currentLine, offset, input, BASM_STRLEN);
            if(!test) return -4;
            storeDataDeclaration(system, dataLabel, input, currentLine, lineNum, storeArray);
        break;

    }

    return !system->cpu.exit;
}


static void cleanDataAddr(ASMSys *system){
    cleanAllData((ASMData*)system->data, &system->dataCount);
    return;
}

//clear and free all memory used by jump point storage
static void cleanJumpPoints(ASMSys *system){
    cleanAllData((ASMData*)system->jumps, &system->jumpCount);
    return;
}


/*
*Converting a program requires two pass throughs of the source file.

*   1st Pass through collects data declarations, converts those to NOP.
*Then jumps are collected and stripped off the line of the target location.
*Both jumps and data are store in a list which is then sorted for use in Pass
*number 2. Cpu operation labels are then stripped and converted to a numerical value
*stored in the first character of the line followed by a space.
*
*   2nd Pass, using sorted lists of data and jumps, combs through the newly refined lines,
*converts any hardware labels, data labels, and then jumps, to an address value
*for super fast parsing later on.
*/

/*
*       Strips the operation label from a line of code
*   then replaces it with its numerical value.
*/
static int opToNumber(ASMSys *system, ASMLineInfo *currentLine, bagDWord lineNum){
    char label[BASM_STRLEN];
    size_t len = 0;

    //grab the operation in line
    char *pos = strpbrk((char*)currentLine->str, " \t\0");
    if(!pos) len = currentLine->len;
    else len = (pos - (char*)currentLine->str);

    if(len > BASM_LABELLEN) len = BASM_LABELLEN;
    memcpy(&label[0], currentLine->str, sizeof(char) * len);
    if(len < BASM_LABELLEN) label[len] = '\0';


    if(len > 0){
        check_Deprecated(system, label);
        bagOpInt tempOpNum = asm_listGetNoCase(system->global->opLabels, system->global->opCount, label);
        if(tempOpNum > -1){
            bagOpInt opNum = system->global->opLabels[tempOpNum].addr;
            
#ifdef USE_FUNC_ADDR
            storeAddress((bagAddrPtr)system->global->op[opNum], (bagUByte*)ASM_LineBuf);
#else
            //copy cpu operation number to line buffer
            memcpy(ASM_LineBuf, &opNum, sizeof(char) * BASM_OPNUM_SIZE);
#endif

            //copy rest of line to a buffer
            while(len < currentLine->len &&
                  (currentLine->str[len] == ' ' || currentLine->str[len] == '\t'))
            {
                    len++;
            }

            if(len < currentLine->len)
                strncpy(&ASM_LineBuf[BASM_INSTRUCT_SIZE], (char*)currentLine->str + len,
                        BASM_STRLEN - BASM_INSTRUCT_SIZE);
            else
                ASM_LineBuf[BASM_INSTRUCT_SIZE] = '\0';

            //replace old line
            size_t newSize = (currentLine->len -len) + BASM_INSTRUCT_SIZE;
            if(ASMLine_Set(currentLine, ASM_LineBuf, (bagWord)newSize)){
                errorMsg(system, lineNum, "Error allocating line: %d\n", lineNum);
                return 0;
            }
        }
        else{
          errorMsg(system, lineNum, "Unknown Operation: \"%s\"\n", label);
          return 0;
        }
    }
    return 1;
}

//Store data declarations, Jump Labels, and convert operations to their op number
//This is done as the file is read, line by line
static int _scanLineData(ASMSys *system, ASMLineInfo *curLine, bagDWord lineNum){
    if(parseLineData(system, curLine, lineNum)){
        //convert cpu operation label to its operation number for faster parsing
        if(!system->prgm.file._skipCurLine && !opToNumber(system, curLine, lineNum)) return 0;//exit on error
        return 1;
    }
    return 0;
}

//replace arguments with their ram addresses
static int compileASMLine(ASMSys *system, ASMLineInfo *currentLine, bagDWord lineNum){

    //Buffer for working on the line
    //directly copy the op number to the line buffer
    bagByte lineBuf[BASM_STRLEN];
    //for(int v = 0; v < BASM_OPNUM_SIZE; v++)
    //    lineBuf[v] = currentLine->str[v];
    memcpy(lineBuf, currentLine->str, sizeof(char) * BASM_INSTRUCT_SIZE);

    if(currentLine->len > BASM_INSTRUCT_SIZE){

        //exit on errors
        if(!varhwToAddr(system, currentLine, lineNum, lineBuf, BASM_STRLEN))
            return 0;

#ifndef USE_FUNC_ADDR
        //Now check if the operation contains its correct arguments
        bagOpInt curOp = *(bagOpInt*)(currentLine->str);

        //check to make sure an operation has its minimum required ops
        if (currentLine->args < system->global->opArgCount[curOp]){
            errorMsg(system, lineNum, "Expected %d parameters, found %d\n",
                     system->global->opArgCount[curOp], currentLine->args);
            return 0;
        }
        
        if(system->global->opLabels[curOp].addr == NOP) return 2;
#endif
    }
    return 1;
}

static int compileASMCode(ASMSys *system, ASMLineInfo *allLines, bagDWord linesCount){

    bagDWord i = 0;
    //some more scanning which requires the sorted lists
    for(i = 0; i < linesCount; i++){
        if(!compileASMLine(system, &allLines[i], i)) return 0;
    }
    return 1;
}


/*
*The following functions are merely for loading the .asm file
*line by line to a buffer and parses for program flags
*/
/*===================================================================================
*   .asm file reading
*
*
*
===================================================================================*/

//Creating, modifying and creating lines within the program
static bagDWord asmFileLineCount(const ASMFile *txtfile){
    return txtfile->line_count;
}

static void asmFileClean(ASMFile *txtfile){
    if(!asmFileLineCount(txtfile) || !txtfile->line){
        txtfile->line = NULL;
        return;
    }

    unsigned int i = 0;
    for(i = 0; i < txtfile->line_count; i++){
        if(txtfile->line[i].str){
            asm_free(txtfile->line[i].str, txtfile->line[i].len, sizeof(char));
        }

        txtfile->line[i].str = NULL;
        txtfile->line[i].len = 0;
        txtfile->line[i].args = 0;
    }
    asm_free(txtfile->line, txtfile->line_count, sizeof(ASMLineInfo));
    txtfile->line = NULL;

    txtfile->line_count = 0;
    return;
}





//Reading program pragma settings
static int getSetting(const char *setting, char *input, bagWord *curSetting){
    size_t newLen = strlen(setting);
    if(!strncmp(input, setting, newLen)){
        size_t fullLen = strlen(input) - 2;//remove any new line character
        if(newLen < fullLen){
            input += newLen;
            (*curSetting) = (bagWord)stringToNumber(input);
            return 1;
        }
        else{
            *curSetting = 1;
            return 1;
        }
    }
    return 0;
}

static int asmFileReadHeader(FILE *file, ASMSys *system){
    //not the basm binary file
    char tempbuf[MAX_PATH];
    int startFound = 0, temp = 0;
    while(!startFound && !feof(file)){
        fgets(tempbuf, MAX_PATH, file);
        //scan top of file for program flags
        if(!strncmp(tempbuf, "_start:", strlen("_start:"))){
            //once _start: is found, we won't search for flags anymore
            startFound++;
            break;
        }

        getSetting("[Debug Text]", tempbuf, &system->settings.debugTxt);//output interpreter debug text while program runs
        getSetting("[Debug FAT]", tempbuf, &system->settings.fatLog);//write debug info to text
        getSetting("[Debug Time]", tempbuf, &system->settings.runTime);
        if(getSetting("[Compile]", tempbuf, &temp)){//output an optimized binary for loading
            SET_FLAG(system->flags, ASM_COMPILE);
        }
        else if(getSetting("[Debug Lines]", tempbuf, &temp)){//dumps file with program lines for debug messages
            SET_FLAG(system->flags, ASM_DUMPLINES);
        }
        else if(getSetting("[Auto Error]", tempbuf, &temp)){//let BAGASMI halt on errors such as missing file
            SET_FLAG(system->flags, ASM_AUTOERROR);
        }
        else if(getSetting("[Strict Registers]", tempbuf, &temp)){//use strict register handling
            SET_FLAG(system->flags, ASM_FORCEREGS);
        }
        #ifndef BASM_USE_STACK_MEM
            else if(getSetting("[RAM Size ", tempbuf, &temp)){
                asmSetRamSize(system, temp);
            }
        #endif

        #if defined(PLATFORM_DS2)
            /*if(getSetting("[CPU_freq ", tempbuf, &system->settings.cpuSpeed)){
                if(system->settings.cpuSpeed)
                    system_setCpu(system);
            }*/
            getSetting("[Flip Mode ", tempbuf, &system->settings.flipMode);
        #endif

        getSetting("[Console ", tempbuf, &system->settings.console);
        //program is designed to be an addon
        getSetting("[Script]", tempbuf, &system->settings.passive);
        //grab the number of args the program needs
        getSetting("[ARGS ", tempbuf, &system->settings.argCount);
    }
    return 1;
}



/*Takes an input line and:
*   strips trailing comments
*   strips leading white spaces
*   stripts trailing white spaces
*/
static size_t _fixLine(char *line){
    //strip off trailing comments
    char *pos = strrchr(line, PARSE_COMMENT);
    if(pos) *pos = '\0';

    pos = strpbrk(line, "\n\r");
    if(pos) *pos = '\0';

    size_t len = strlen(line),
           bufSize = len;
    //strip pre spaces and tabs
    int z = 0;
    while(z < len && (line[z] <= 32 || line[z] >= 127))line[z++]='\0';
    if(z >= len) return 0;//blank line?
    else if(z > 0) {
        //strncpy(line, line+z, bufSize);
        memcpy(line, line+z, sizeof(char) * (bufSize - z));
        len -= z;//subtract the removed whitespace
        line[len] = '\0';//append null to chop off extra characters
    }
    //strip trailing spaces
    while(len > 0 && (line[len] <= 32 || line[len] >= 127) && line[len] != '\0') line[len--] = '\0';
    if(len <= 0) {
        line[0] = '\0';
        return 0;
    }
    len++;

    //after stripping pre-spaces check if we still have a comment
    if(line[0] == PARSE_COMMENT){
        line[0] = '\0';
        len = 0;
    }
    return len;
}

//read a file and store all jump points and data declarations
static int asmReadLine(ASMSys *system, ASMLineInfo *newLine, char *inLine, bagDWord linesCount){
    //check for blank line
    size_t newLen = _fixLine(inLine);
    if(!newLen) system->prgm.file._skipCurLine++;
    else {
        //then allocate space on the current line to store the current operation
        if(ASMLine_Set(newLine, inLine, (bagWord)-newLen)){
            errorMsg(system,  -1, "Failed to allocate memory for line:\n\"%s\"\n", inLine);
            return -1;
        }
        //scan line for data declarations or jump points
        if(!_scanLineData(system, newLine, linesCount)){
            errorMsg(system,  -1, "Error scanning line for data:\n\"%s\"\n", inLine);
            return -2;
        }
    }
    return 0;
}

//ToDo: Add better return value handling
static int asmPushLine(ASMSys *system, char *line, bagDWord lineNum){
	ASMFile *txtfile = &system->prgm.file;
	ASMLineInfo readLine = {};

	int result = asmReadLine(system, &readLine, line, lineNum);
	if(result == 0 && !txtfile->_skipCurLine){
	    //allocate buffers to hold formatted string
	    ASMLineInfo *tempLine = NULL;

        asm_realloc(tempLine, txtfile->line,
                    (lineNum + 1) * sizeof(ASMLineInfo),
                    lineNum * sizeof(ASMLineInfo));

        if(!tempLine){
            errorMsg(system, -1, "Error reallocing prgm lines\n");
            return 99;
        }
        txtfile->line = tempLine;
        txtfile->line_count = lineNum + 1;
        memcpy(&txtfile->line[lineNum], &readLine, sizeof(ASMLineInfo));
	}
    else if (result < 0 || txtfile->_skipCurLine){
        //some sort of error occured, free up line memory
        ASMLine_Clear(&readLine);
        //flag this line for skipping
        txtfile->_skipCurLine++;
    }
	return result;
}


static int asmFileRead(const char *filepath, ASMSys *system){
    //clean up jumps and data declarations
    cleanDataAddr(system);
    cleanJumpPoints(system);

    //clean up the text file first
    ASMFile *txtfile = &system->prgm.file;
    txtfile->line_count = 0;
    asmFileClean(txtfile);


    //if we want debug lines, then open the file to write
    //the processed lines to.
    #if defined(OUTPUT_LINES_FILE)
        FILE *lineDump = NULL;

        if(GET_FLAG(system->flags, ASM_DUMPLINES))
            lineDump = openLineDump(filepath, "wb");
    #endif

    //open the text file
    FILE * file = fopen(filepath, "rb");
    if(file == NULL)
        return -1;

    
    bagDWord count = asmFileLineCount(txtfile);//hopefully set to 0 here, or maybe we are importing more code!
    int err = 0, startFound = 0, end = 0;
    char *line = NULL;

    char tempbuf[MAX_PATH];
    while(!end){
        ///scan file for executable lines and format for copying
        line = NULL;
        if(!feof(file))
            line = fgets(tempbuf, MAX_PATH, file);
        else{
            int v = 0;
            //this oughta work
            tempbuf[v++] = 'H';tempbuf[v++] = 'A';tempbuf[v++] = 'L';tempbuf[v++] = 'T';
            for(;v<MAX_PATH;v++)tempbuf[v] = '\0';
            line = tempbuf;
            end++;
        }

        if(startFound == 0){
            //scan top of file for program flags
            if(!strncmp(tempbuf, "_start:", strlen("_start:")))//once _start: is found, we won't search for flags anymore
                startFound++;
        }
        else
            startFound++;//skip start found line

        //skip blank lines and comments
        if(!startFound || !line ||  line[0] == PARSE_COMMENT)
            continue;


        //write out processed line for debug purposes
        #if defined(OUTPUT_LINES_FILE)
            if(lineDump){
                if(_fixLine(line) > 0)
                    fprintf(lineDump, "%s\n", line);
            }
        #endif

        //push a line into the program buffer
        //ToDo:Add better return value handling
        system->prgm.file._skipCurLine = 0; //reset back to default
        err = asmPushLine(system, line, count);
        if(system->prgm.file._skipCurLine) continue; //skip line
        else if(err < 0) break;
        count++;
    }

    //close the debug file
    #if defined(OUTPUT_LINES_FILE)
        if(lineDump) fclose(lineDump);
        lineDump = NULL;
    #endif

    //mose the main text file
    txtfile->line_count = count;
    fclose(file);

    if(err == 0){//no errors
        //then sort lists for faster searching
        asm_listSort(system->data, system->dataCount);
        asm_listSort(system->jumps, system->jumpCount);

        return 1;
    }
    return err;
}


/*===================================================================================
*   .basm binary reading/writing
*
*
*
===================================================================================*/
//Converts addresses stored by the vm into relative addresses for each instruction.
//That is, all ram addresses just subtract the address of where the vm memory is
//And all register addresses will subtract off the first register position.
static int calcRelativeAddr(ASMSys *sys, ASMLineInfo *curLine, bagUByte *newLine) {
    //first copy instruction operation to the line
    int pos = 0;
    for (; pos < BASM_INSTRUCT_SIZE; pos++) newLine[pos] = curLine->str[pos];
    
    //next, go through the arguments and fix their addresses
    int i = 0;
    bagByte *line = &curLine->str[BASM_INSTRUCT_SIZE];
    bagUByte *outLine = &newLine[BASM_INSTRUCT_SIZE];
    
    bagAddrPtr ramStart = (bagAddrPtr)&sys->memory.byte[0],
                ramEnd   = (bagAddrPtr)&sys->memory.byte[BASM_MEMSIZE - 1],
                regStart = (bagAddrPtr)&sys->cpu.reg[0],
                regEnd   = (bagAddrPtr)&sys->cpu.reg[BASM_REGCOUNT - 1];
    
    while(i < curLine->args){
        //read input address
        bagAddrPtr curAddr = (bagAddrPtr)readAddress((bagMemInt*)line);

        //writing addresses
        if (curAddr >= ramStart && curAddr <= ramEnd){
            curAddr -= ramStart;
            curAddr |= BASMC_ADDR_RAM;
        } else if(curAddr >= regStart && curAddr <= regEnd) {
            curAddr -= regStart;
            curAddr |= BASMC_ADDR_REG;
        }
        //loading addresses
        else if(curAddr & BASMC_ADDR_REG) {
            curAddr &= ~BASMC_ADDR_REG;
            curAddr += regStart;
        } else if (curAddr & BASMC_ADDR_RAM){
            curAddr &= ~BASMC_ADDR_RAM;
            curAddr += ramStart;
        } else {
            printf("unknown address\n");
        }
        
        //store new value again
        storeAddress(curAddr, (bagMemInt*)outLine);
        line += (BASM_ARG_SIZE);
        outLine += (BASM_ARG_SIZE);
        i++;
    }
    return 1;
}


/*
*Load the optimized binary file
*/
//reads program settings from binary header
static int readBinaryHeader(FILE *binFile, ASMSys *system){
    ASMHeader header = {};
    fread(&header, 1, sizeof(ASMHeader), binFile);

    if(header.platform != COMPILED_FOR){
        printFunction(system, "Wrong platform binary!\n");
        return -1;
    }

    if(header.version != BAGASM_VER){
        printFunction(system,
                    "Unsupported code version.\nRequires version %d, current version: %d\n",
                    header.version, BAGASM_VER);
        return -1;
    }

    //system->settings.debugTxt = 1;

    system->settings.console = header.console;
    system->settings.totalRam = (bagWord)header.totalRam;
    system->settings.passive = header.script;
    system->settings.screenWidth = header.screenWd;
    system->settings.screenHeight = header.screenHt;
    #if defined(PLATFORM_DS2)
        system->settings.flipMode = header.flip;
    #endif
    system->settings.argCount = header.argCount;
    return 1;

}

//Read a binary file previously output through BAGASMI
static int loadBinary(const char *binPath, ASMSys *system){
    FILE *inFile = fopen(binPath, "rb");
    if(!inFile){
        printFunction(system, "Error opening binary\n");
        return 0;
    }
#ifdef DBGTEXT
    if(system->settings.debugTxt)
        printFunction(system, "Reading binary\n");
#endif

    //need a bit more from the binary header to load the file
    ASMHeader header = {};
    fread(&header, 1, sizeof(ASMHeader), inFile);
    //read ram dump
    if(header.ramSize > 0){
#ifdef DBGTEXT
        if(system->settings.debugTxt) printFunction(system, "Reading ram dump\n");
#endif
        system->memory.pos = header.ramSize;
        fseek (inFile, header.ram, SEEK_SET);
        fread(&system->memory.byte, 1, system->memory.pos, inFile);
    }

    //now read rest of program
    fseek(inFile, header.prgm, SEEK_SET);

    //allocate space for lines
    ASMFile *prgmFile = &system->prgm.file;

    prgmFile->line_count = header.lines;
#ifdef DBGTEXT
    if(system->settings.debugTxt)
        printFunction(system, "line count: %d\n", prgmFile->line_count);
#endif
    asm_calloc(prgmFile->line, sizeof(ASMLineInfo), prgmFile->line_count);
    if(!prgmFile->line){
        errorMsg(system, -1, "Error allocating binary space\n");
        return 0;
    }

    //now loop through each line and read the data
    for(int i = 0; i < prgmFile->line_count; i++){
        ASMLineInfo *curLine = &prgmFile->line[i];
        //first read arg count
        fread(&curLine->args, 1, header.lenSize, inFile);
#ifdef DBGTEXT
        if(system->settings.debugTxt)
            printFunction(system, "args: %d ", curLine->args);
#endif
        //then read line size
        fread(&curLine->len, 1, header.lenSize, inFile);
#ifdef DBGTEXT
        if(system->settings.debugTxt)
            printFunction(system, "len: %d\n", curLine->len);
#endif
        //then allocate and read line
        asm_calloc(curLine->str, 1, (sizeof(char) * curLine->len));
        if(!curLine->str){
            errorMsg(system, -1, "Error allocating line space\n");
            return 0;
        }
        fread(curLine->str, 1, curLine->len, inFile);


        unsigned char tempLine[MAX_PATH];
        //add new ram address
        calcRelativeAddr(system, curLine, tempLine);
        memcpy(curLine->str, tempLine, sizeof(char) * curLine->len);
    }
    fclose(inFile);
    ASM_InitARGV(system, system->settings.argCount,
                    (char**)system->passed_args.argv);
    return 1;
}

/*
*Dump the optimized file into a binary file to load for later
*/
static int dumpBinary(const char *outFile, ASMSys *system){
#ifdef DBGTEXT
    if(system->settings.debugTxt)
        printFunction(system, "Writing binary;\n");
#endif

    ASMFile *prgm = &system->prgm.file;
    if(asmFileLineCount(prgm) == 0){
        printFunction(system, "No lines to write!\n");
        return 0;
    }

    FILE *dest = fopen(outFile, "wb");
    if(!dest){
        printFunction(system, "Failed to open output file\n");
        return -1;
    }

    //create the binary header
    ASMHeader header = {};
    strcpy((char*)header.magicNumber, ".BASM");
    header.platform = COMPILED_FOR;
    header.version = BAGASM_VER;

    header.ram = sizeof(ASMHeader);
    header.ramSize = system->memory.pos;//last memory position
    header.totalRam = system->settings.totalRam;//how much ram was allocated for the system
    header.prgm = header.ram + header.ramSize;//code starts right after header
    header.lenSize = sizeof(prgm->line->args);//size of short

    //program settings
#if defined(PLATFORM_DS2)
    header.flip = system->settings.flipMode;
#endif
    header.console = system->settings.console;//if console is enabled on DS
    header.script = system->settings.passive;

    header.screenWd = system->settings.screenWidth;
    header.screenHt = system->settings.screenHeight;

    header.lines = (int)asmFileLineCount(prgm);
    header.argCount = system->settings.argCount;

    //now to write the header info
    fwrite(&header, 1, sizeof(ASMHeader), dest);

    //dump the initialized ram state. Variables and constant values will be
    //stored in here
    if(header.ramSize > 0)
        fwrite(&system->memory.byte, sizeof(bagMemInt),
                 header.ramSize, dest);

    //now to write each line of code
    for(int i = 0; i < header.lines; i++){
        //make sure the line is allocated
        ASMLineInfo *curLine = &prgm->line[i];
        if( curLine == NULL)
            break;

        //write info about the line
        //first how many args there are
        fwrite(&curLine->args, 1, sizeof(curLine->args), dest);
        //then the line length
        fwrite(&curLine->len, 1, sizeof(curLine->args), dest);
        //finally, the line itself

        unsigned char tempLine[MAX_PATH];
        //add new ram address
        calcRelativeAddr(system, curLine, tempLine);
        fwrite(tempLine, 1, curLine->len, dest);
    }
    fclose(dest);
    return 1;
}


//output the optimized code to a binary for faster loading in future run times (if flag set in source)
static int _makeBinFile(const char *path, ASMSys *system){
#ifdef DBGTEXT
    if(system->settings.debugTxt)
        printFunction(system, "Creating binary file\n");
#endif
    char newName[MAX_PATH];
    strcpy(newName, path);
    size_t len = strlen(newName);
    while(len > 0 && newName[len] != '.')len--;
    strcpy(&newName[len], ".basm");
    return dumpBinary(newName, system);
}

/*===================================================================================
*   File Loading
*
*
*
===================================================================================*/

//returns if input file is a binary or .asm file
static int _isBinFile(const char *path, ASMSys *system){
    char magicNumber[8];
    FILE *temp = fopen(path, "rb");
    if(temp == NULL){
        printFunction(system, "no file!\n");
        return -1;
    }
    fread(&magicNumber, 1, sizeof(magicNumber), temp);
    fclose(temp);

    return !strcmp(magicNumber, ".BASM");
}

static int asmFileReadSettings(const char *filepath, ASMSys *system){
    FILE * file = fopen(filepath, "rb");
    if(file == NULL){
        printFunction(system, "null file!\n");
        return -1;
    }

    int success = 1;

    //check if we are reading a binary file
    char magicNumber[8];
    fread(&magicNumber, 1, sizeof(magicNumber), file);
    fseek(file, 0, SEEK_SET);

    //if we aren't, then read normal settings
    if(strcmp(magicNumber, ".BASM"))
        success = asmFileReadHeader(file, system);
    //otherwise we need to grab info from header
    else
        success = readBinaryHeader(file, system);


    fclose(file);
    return success;
}



//load a file to memory and prepare it for execution
static int asmloadFile(ASMSys *system, const char *file){
    if(file == NULL){
        printFunction(system, "No input file specified!\n");
        return -1;
    }
#if defined(OUTPUT_LINES_FILE)
    FilePath_Clean(&system->prgmPath);
    if(!FilePath_Convert(&system->prgmPath, file, (int)strlen(file))){
        errorMsg(system, -1, "Error obtaining program path\n");
        return -1;
    }
#endif
    //continue loading program
    system->prgm.pos = 0;//set the program to start on line 0

    if(_isBinFile(file, system)){
        //the .basm binary is a dump of an already optimized .asm file
#ifdef DBGTEXT
        if(system->settings.debugTxt) printFunction(system, "loading binary\n");
#endif
        //no work needs to be done to convert it
        int test = loadBinary(file, system);
        if(test != 1)
            return test;
    }
    //otherwise we are loading a regular script file
    else{
        system->prgm.file.line_count = 0;
        int err = asmFileRead(file, system);
        if(err < 1) return err;

        if(!compileASMCode(system, system->prgm.file.line, system->prgm.file.line_count))
            return -1;

#if defined(DBGTEXT)
        if(system->settings.debugTxt){
            printFunction(system, "File compiled!\nDoing some cleanup...\n");
        }
#endif

        //make binary file if requested
        if(GET_FLAG(system->flags, ASM_COMPILE))
            _makeBinFile(file, system);

        //clean up jump and data address here
        if(GET_FLAG(system->flags, ASM_JUMPPURGE))
            cleanJumpPoints(system);

        if(GET_FLAG(system->flags, ASM_DATAPURGE))
            cleanDataAddr(system);
    }

#if defined(DBGTEXT)
        if(system->settings.debugTxt){
            printFunction(system, "Ready to execute!\n");
        }
#endif
    return 1;
}


/*===================================================================================
*   Initialize and run the system
*
*
*
===================================================================================*/
static int _makeRoot(const char *filePath, ASMSys *system){
    if(!FilePath_Convert(&system->rootPath, filePath, (int)strlen(filePath))){
        errorMsg(system, -1, "failed to allocate root path!\n");
        return -1;
    }
    return 0;
}

static void _cleanRoot(ASMSys *system){
   /* if(system->curRoot){
        asm_free(system->curRoot, strlen(system->curRoot) + 1, sizeof(char));
        system->curRoot = NULL;
    }*/
    FilePath_Clean(&system->rootPath);
    return;
}

/*
*Clean all memory used by the system
*/
void ASM_CleanSystem(ASMSys *system){
    if(system->cleaned) return;
    system->cleaned = 1;

#if defined(BASM_COMPILE_FAT)
    //close any open file handles
    for(int i = 0; i < BASM_FILEHANDLE_COUNT; i++){
        if(system->fh[i]) fclose(system->fh[i]);
        system->fh[i] = NULL;
    }
#endif

    ASM_cleanCpuOps(system->global);
    ASM_ClearArgs(system);
    asmFileClean(&system->prgm.file);

    cleanDataAddr(system);
    cleanJumpPoints(system);
    asmCleanRam(system);

#ifdef BASM_COMPILE_VIDEO
    ASM_freeVideo(system);
#endif

#ifdef BASM_COMPILE_EXTLIBS
    ASM_freeExtLib(system);
#endif

    _cleanRoot(system);

#if defined(OUTPUT_LINES_FILE)
    if(GET_FLAG(system->flags, ASM_DUMPLINES)){
        char tempPath[MAX_PATH];
        if(FilePath_Export(&system->prgmPath, tempPath, MAX_PATH)){
            removeLineDump(tempPath);
        } else {
            errorMsg(system, -1, "Error exporting prgm path\n");
        }
        FilePath_Clean(&system->prgmPath);
    }
#endif

    //if (ASM_MEMUSED >= sizeof(ASMSys))
    ASM_MEMUSED -= sizeof(ASMSys);
    return;
}


#if defined(DUMP_OPS_LIST)
static int asm_ExportOpsList(ASMSys *system){
    char rootPath[MAX_PATH];
    FilePath_Export(&system->rootPath, rootPath, MAX_PATH);

    char filePath[MAX_PATH];
    sprintf(filePath, "%sopslog.txt", rootPath);



    printFunction(system, "Dumping ops list to:\n\t%s\n", filePath);
    FILE *outfile = fopen(filePath, "wb");
    if(!outfile){
        printFunction(system, "Error outputting ops\n");
        return 0;
    }
    for (int i = 0; i < system->global->opCount; i++)
        fprintf(outfile, "%s (%d)\n", system->global->opLabels[i].label, system->global->opLabels[i].addr);
    fclose(outfile);
    return 1;
}
#endif


//Assumes the system has already been initialized
void ASM_ResetSys(ASMSys *system) {
    system->cpu.exit = 0;
    system->prgm.pos = 0;
    system->cpu.curVal = 0;
    system->cpu.status = 0;
    system->cpu.curOp = 0;
    system->cleaned = 0;
    system->prgm.jmpBackPos.start = 0;
    system->prgm.jmpBackPos.end = system->prgm.jmpBackPos.total;
    system->prgm.jmpBackPos.cur = 0;
    
    for(int i = 0; i < BASM_REGCOUNT; i++)
        system->cpu.reg[i] = 0;
    
    //initialize stack register to the proper address
#if defined(BASM_ENABLE_STACK)
    system->cpu.reg[REG_SP] = (bagAddrPtr)&system->memory.stack[BASM_STACKSIZE-1];
    memset(system->memory.stack, 0, BASM_STACKSIZE);
    //printf("Stack addr: %p\n", (bagAddrPtr*)system->cpu.reg[REG_SP]);
#endif
    
#if defined(BASM_MEMMAPPED_IO)
    system->cpu.reg[REG_MEMIO] = (bagAddrPtr)&system->memory.memio[0];
#endif
    
    if(!system->passed_args.allocated){
        for(int i = 0; i < BASM_MAX_ARGS; i++)
            system->cpu.arg[i] = NULL;
    }
    

    return;
}

//if file is NULL, we can initialize a system for a line by line by line interpretation
char ASM_InitSystem(const char *file, ASMSys *system, int flags){
    //read file settings first for initialization instructions
#if defined(DBGTEXT)
    printFunction(system,"Initializing System...\n");
#endif
    //calculate the memory of this system = new mem - prev mem used
    bagUWord oldMem = ASM_MEMUSED;

    //exit first if ARGs didn't properly setup
    if(system->cpu.exit == BASM_EXIT_ERROR)
        return -1;

    //initialize some system variables
    system->cleaned = 0;
    system->flags = flags;
    system->global = &ASM_Globals;//use global operations
    system->settings.console = BASM_DEFAULT_CONSOLE;
    asmSetRamSize(system, 0);
    system->settings.screenWidth = system->settings.screenHeight = 0;
  
    //initialize ring buffer for storing jump backs
    ringBuf_init(&system->prgm.jmpBackPos, BASM_JUMPBACK_COUNT);

#if defined(BASM_COMPILE_FAT)
    //reset file handles
    for(int i = 0; i < BASM_FILEHANDLE_COUNT; i++){
        system->fh[i] = NULL;
    }
#endif

    //get the root path of the launched program
    if(!GET_FLAG(system->flags, ASM_LINEBYLINE)){
	    _getFilePath(file, ASM_LineBuf, sizeof(ASM_LineBuf));
	} else {
        strncpy(ASM_LineBuf, "./\0", 3);
    }

    if(_makeRoot(ASM_LineBuf, system))
        return -1;



    //read file settings first for initialization instructions
#if defined(DBGTEXT)
    if(system->settings.debugTxt)
        printFunction(system,"Reading program settings...\n");
#endif


    //read file pragmas first to see how to initialize the system
    if(file && asmFileReadSettings(file, system) <=0){
        //errorMsg(system, -1, "Error reading source pragmas.\n");
        //return -1;
    }

#if defined(DBGTEXT)
    if(system->settings.debugTxt)
        printFunction(system,"Initializing CPU Ops\n");
#endif

    //initialize standard operations
    ASM_InitSTDOps(system);

#if defined(DBGTEXT)
    if(system->settings.debugTxt)
        printFunction(system,"Initializing ram...\n");
#endif

    //initialize ram
    if(!asmInitRam(system)){
        errorMsg(system, -1, "Error allocating RAM.");
        return -1;
    }
    //copy launch args to ram from temp ram
    asm_pushArgs(system);

    //initiate input if enabled
#if defined(BASM_COMPILE_INPUT)
    ASM_initInput(system);
#endif

#if defined(DBGTEXT)
    if(system->settings.debugTxt)
        printFunction(system,"Initializing video...\n");
#endif

    //initiate video if enabled
#if defined(BASM_COMPILE_VIDEO)
    if(!ASM_videoInit(system))
        return 0;
#endif

    //initialize external libraries
#if defined(BASM_COMPILE_EXTLIBS)
    if(!ASM_extLibInit(system))
        return 0;
#endif
#if defined(DBGTEXT)
    if(system->settings.debugTxt)
        printFunction(system,"External Libs Done!\n");
#endif
    
    //sort collected operations for faster searching
    if(system->global->stdOpInit == 1)//only ssort if this is the first time initializing
        asm_listSortNoCase(system->global->opLabels, system->global->opCount);

    

#if defined(DUMP_OPS_LIST)
    asm_ExportOpsList(system);
#endif


    //load the script file now
    if(file){
#if defined(DBGTEXT)
        //printFunction(system,"Reading program file...\n");
#endif
	    int err = asmloadFile(system, file);
	    if(err < 1){
	        errorMsg(system, -1, "Error loading file: \"%s\"\n", file);
	        return err;
	    }
	}

    ASM_ResetSys(system);
    
    //update memory used by the system
    ASM_MEMUSED += sizeof(ASMSys);
#if defined(DBGTEXT)
    if(system->settings.debugTxt)
        printFunction(system,"System fully initialized\n");
#endif

    system->memUsed = ASM_MEMUSED - oldMem;
    return 1;
}



//Converts the input script file and outputs a
//compiled binary file to run in the interpreter
int ASM_BuildFile(ASMSys *system, const char *file){
    int status = ASM_InitSystem(file, system, ASM_DATAPURGE | ASM_JUMPPURGE | ASM_COMPILE);
    ASM_CleanSystem(system);
    return status;
}

//ToDo: add better return value checking for asmPushLine
//ToDo: add ability to call a line execution from within a script
/*
	Compiles and executes a single line of code supplied by "input". Code
	is stored in the system->prgm.file.lines buffer,so it assumes there is
	no previous code before the first execution of this function.

	pushLines and pushDepth are used for collecting lines within loops
	beginning with LOOPTO and ending with LOOPBACK. NULL for either one
	of these values will disable the line caching in loops and execute
	line by line as normal.
*/

int ASM_ExecuteLine(ASMSys *system, char *input, int *pushLines, int *pushDepth){

	int error = 0;
	bagDWord pos = system->prgm.pos + (*pushLines);

	//collect line and scan for data within it
	//stores line in system->prgm.file.line buffer
    system->prgm.file._skipCurLine = 0;
	error = asmPushLine(system, input, pos);
	if(system->prgm.file._skipCurLine && error != 99) goto EXIT; //skip line
	else if(error < 0) {
		error = 2;
		goto EXIT;
	}

	//sort data collected
    asm_listSort(system->data, system->dataCount);
    asm_listSort(system->jumps, system->jumpCount);

    ASMLineInfo *curLine = &system->prgm.file.line[pos];
    if(!compileASMLine(system, curLine, pos)){
    	error = 3;
    	goto EXIT;
    }

    /*If we are doing loop code, just push the lines of code until
      the end of loop is detected. Then execute the loop code
    */
    if(pushDepth && pushLines){
	    extern int ASM_isLoopOp(ASMSys *system, bagOpInt op);
	    bagOpInt curOp = *(bagOpInt*)(curLine->str);

	    int test = ASM_isLoopOp(system, curOp);
		if (test == 2){
			(*pushDepth)--;
	    }
	    if( test == 1 || (*pushDepth) > 0){
	    	(*pushLines)++;
	    	(*pushDepth) += (test == 1);
	    	return 2;
	    }
	    else if(*pushDepth == 0) (*pushLines) = 0;
	}

    //execute all lines until the end of file
    while(system->prgm.pos < system->prgm.file.line_count){
	    if(!ASM_Step(system)){
			if(system->cpu.exit == BASM_EXIT_CLEAN) error = -1;
			else error = 4;
		}
	}
    
	EXIT:
		//if any major errors, just replace the added line with NOP
		if(error >= 2){
			if (error == 4) system->prgm.pos--;
			if(system->prgm.file.line_count > system->prgm.pos){
				ASMLine_Clear(&system->prgm.file.line[system->prgm.pos]);
            }
		}
		return error;
}

void asm_terminalLoop(ASMSys *system){
	char input[MAX_PATH];
	int pushLines = 0, pushDepth = 0;
	while(1){
		system->cpu.exit = 0;

		//small prompt for user input
		if (!pushDepth ) BASM_PRINT(">>> ");
		else BASM_PRINT ("... ");//lines to be executed in loop

		//get user input
		//char *kb = gets(input);
        char *kb = fgets(input, MAX_PATH, stdin);
		if(!kb){ //detect end of line
			strcpy(input, "HALT");
		}

		int val = ASM_ExecuteLine(system, input, &pushLines, &pushDepth);

		if(val >= 2)
			continue;
		else if (val < 0)
			return;
	}

	//system->prgm.file.line_count++;
	return;
}


//default function for running an asm file
int ASM_run(ASMSys *system, const char *file){

    int flags = ASM_DATAPURGE | ASM_JUMPPURGE;
    //if no file is selected, then we can just line by line execute
    if(!file) flags |= ASM_LINEBYLINE;

    /*first initiate the system, we are purging all variable and jump declarations
    as they aren't needed after optimization.
    However the is the choice to keep the sorted name lists in memory if one
    so desires to use the interpreter for other means in their own projects.
    */
    if(ASM_InitSystem(file, system, flags) != 1)
        goto EXIT;

    
    PrintBanner(system, Platform);

    printFunction(system, "Running Program:\n*****************************\n\n\n");

    //if runtime is defined, we will time the program as it runs
#if defined(RUNTIME)
    u32 timer = 0;
    static float finalTime = 0;
    if(system->settings.runTime){
        Timer_start(&timer);
    }
#endif

    //main asm interpreting loop
#if defined(D___SYS_DS2_)
    //just a quick loop on the DS2
    while(ASM_Step(system));
#else
    	//if a script file has been supplied, work through it as normal
    	if(!GET_FLAG(system->flags, ASM_LINEBYLINE))
    		while(ASM_Step(system));

    	//otherwise, enter terminal mode where users can immediately execute typed code
    	//line by line
    	else
    		asm_terminalLoop(system);
#endif

#if defined(RUNTIME)
    if(system->settings.runTime){
        finalTime = Timer_getTicks(&timer);
    }
#endif

    //if on the DS or gba, the console needs to be initialized
    //to output run time information
#if defined (PLATFORM_DS) || (PLATFORM_GBA)
    extern void ds_initConsole(void);
    if(!system->settings.console)
        ds_initConsole();
#endif
    printFunction(system, "\n\n\n*****************************\nProgram Complete...\n");

    EXIT:
    //check if the program exited with errors
    //if(system->cpu.exit == BASM_EXIT_ERROR)
    //    printFunction(system, "Exiting with errors!\n");

    //print the time it took for the program to fully complete
#if defined(RUNTIME)
    if(system->settings.runTime){
        char timeText[16];
        sprintf(timeText, "%04f", finalTime);
        printFunction(system, "Program completed in %s s\n", timeText);
    }
#endif

    //output the total operations executed in program
#if defined(COUNT_OPERATIONS)
    printFunction(system, "%u total operations executed\n", (unsigned long)system->settings.operations);
#endif

    printFunction(system, "mem used: %u\n", system->memUsed);
    //free and clean all memory allocations used
    ASM_CleanSystem(system);
    printFunction(system, "left over: %d\n", ASM_MEMUSED);

    return 0;
}

