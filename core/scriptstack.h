#ifndef _SCRIPTSTACK_H_
#define _SCRIPTSTACK_H_

#include "bagasmi.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(USE_SCRIPT_STACK)
    
#define SCRIPT_STACK_MAXSIZE 10

typedef enum {
	SCRIPT_N                    = 0,
	SCRIPT_C                    = (1<<1),//cached
	SCRIPT_S                    = (1<<2),//system
	SCRIPT_T                    = (1<<3),//temporary (cleared when batch processed)
    SCRIPT_M                    = (1<<4),//Allow script to run in system menus
    SCRIPT_RETURNTOPARENT       = (1<<5),//Return to parent script when child script finishes
}SCRIPT_MODES;

//Process instance of bagasmi script
typedef struct _asm_script_ {
    //keep track of the script that called the process
    ASMSys *parent, *process;
    char *name, mode, child;
} ASMProcess;


//scripts are processed in order of stack
typedef struct _asm_script_stack {
    ASMProcess *list[SCRIPT_STACK_MAXSIZE];
    int loaded;
    bagUWord memUsed;
    int stackPointer;
} ASMProcess_Stack;



extern void System_ScriptsInit(void);

//swap script position in the stack
extern void System_SwapScripts(int src, int dest);

//push a script onto the stack with arguments
extern int System_PushScript(const char *file, ASMSys *parentScript, int mode, char **argv, int argc, ...);

//end and remove/cleanup script from stack
extern void System_PopScript(int pos);

//process loop for all scripts in stack
extern void System_ProcessScripts(int mode);

//number of scripts running
extern int System_ScriptsRunning(void);

//set the script stack position
    extern void System_TriggerReprocessStack(void);


#ifdef __cplusplus
}
#endif

#endif

#endif
