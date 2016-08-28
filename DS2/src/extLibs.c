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

#include "../../core/extLibs.h"
#include "../../core/scriptstack.h"

#if defined(BASM_COMPILE_EXTLIBS)
/**************************************************************************************
INI file reading/writing
**************************************************************************************/
static void _iniOpen(ASMSys *system){
	//arguements
	char *path = (char*)system->cpu.arg[0];

	//convert path to something useable
	char newLine[MAX_PATH];
	//if an absolute path was given, use it
	BASM_CheckPathLocality(system, path, newLine, MAX_PATH);

	//initialize and load the ini file
	initFastIni(&system->iniFile);

	//ini initialization failed
	if(!system->iniFile.load){
		SET_FLAG(system->cpu.status, BIT_X);
		system->cpu.curVal = 0;
		return;
	}

	if(system->iniFile.load(&system->iniFile, newLine, INI_SORT | INI_ADDMISSING) < 0){
  		if(GET_FLAG(system->flags, ASM_AUTOERROR)){
			errorMsg(system, system->prgm.pos,
				 "Error opening ini file:\n%s\n", newLine);
	  	} else {
			system->cpu.curVal = 0;//return false for error
			SET_FLAG(system->cpu.status, BIT_X);
	  	}
	  	return;
	}
	system->cpu.curVal = 1;//opened successfully
}

static void _iniClose(ASMSys *system){
	if(system->iniFile.close){
		system->iniFile.close(&system->iniFile);
  	}
}

static void _iniWrite(ASMSys *system){
  	if(system->iniFile.write){
		//arguements
		char *path = (char*)system->cpu.arg[0];

		//convert path to something useable
		char newLine[MAX_PATH];
		//if an absolute path was given, use it
		BASM_CheckPathLocality(system, path, newLine, MAX_PATH);
		//arguments
		//output filepath
		system->iniFile.write(&system->iniFile, newLine);
  	}
}

//read integer value from the current open ini file
static void _iniReadInt(ASMSys *system){
	if(system->iniFile.getInt){
		//arguments
		//storage, section, name, default value
		system->cpu.status = (*system->cpu.arg[0]) = system->iniFile.getInt(&system->iniFile, (char*)system->cpu.arg[1],
													  (char*)system->cpu.arg[2], (int)(*system->cpu.arg[3]));

		system->cpu.curVal = 1;
	}
}

//set an integer value in an ini file
static void _iniSetInt(ASMSys *system){
	if(system->iniFile.setInt){
		//arguments
		//section, name, new value
		system->iniFile.setInt(&system->iniFile, (char*)system->cpu.arg[0], (char*)system->cpu.arg[1], (int)*system->cpu.arg[2]);
		system->cpu.curVal = 1;
	}
}

//read a string value from the current open ini
static void _iniReadStr(ASMSys *system){
  	//arguments
  	//storage, storage size, section, name, defaultStr
  	//storage, read string, storage size
  	strncpy((char*)system->cpu.arg[0],
		  	//the value to copy
		  	system->iniFile.getStr(&system->iniFile, (char*)system->cpu.arg[2],
								  (char*)system->cpu.arg[3], (char*)system->cpu.arg[4]),
		  	//size
		  	(int)*system->cpu.arg[1]
	);

  	system->cpu.curVal = 1;
}

static void _iniSetStr(ASMSys *system){
	if(system->iniFile.setStr){
		//arguments
		//section, name, new str
		system->iniFile.setStr(&system->iniFile, (char*)system->cpu.arg[0], (char*)system->cpu.arg[1], (char*)system->cpu.arg[2]);
		system->cpu.curVal = 1;
	}
}


#if defined(USE_SCRIPT_STACK)
/**************************************************************************************
Loading Scripts within scripts!
 **************************************************************************************/
static void _scriptExecute(ASMSys *system) {
    extern void ASM_SystemYield(ASMSys *system);
    
    
    //count how many arguments there are
    int argc = -1; //start at negative one to counter the script file path argument
    while(argc < BASM_MAX_ARGS && system->cpu.arg[argc]) argc++;

	char newLine[MAX_PATH];
	//if an absolute path was given, use it
	BASM_CheckPathLocality(system, (char*)system->cpu.arg[0], newLine, MAX_PATH);
    if(argc > 0) {
        char *argv[argc];
        if(argc > 0) {
            for(int i = 0; i < argc; i++) argv[i] = (char*)system->cpu.arg[i + 1];
        }
    
        //filename, the following arges, are argv values supplied directly to the script
        System_PushScript(newLine, system, SCRIPT_RETURNTOPARENT, argv, argc);
    } else {
        System_PushScript(newLine, system, SCRIPT_RETURNTOPARENT, NULL, 0);
    }
    //execute the newest push script next
    System_TriggerReprocessStack();
    ASM_SystemYield(system);
}
#endif



/**************************************************************************************
Mapping operations to functions

**************************************************************************************/
static void ASM_initExtLibOps(ASMSys *system){
  	system->global->extLibInit++;
  	if(system->global->extLibInit > 1) return;

  	ASM_setCpuOpsEx(system->global, "INI_OPEN", 1, &_iniOpen);
  	ASM_setCpuOpsEx(system->global, "INI_CLOSE", 0, &_iniClose);
  	ASM_setCpuOpsEx(system->global, "INI_GETINT", 4, &_iniReadInt);
  	ASM_setCpuOpsEx(system->global, "INI_SETINT", 3, &_iniSetInt);
  	ASM_setCpuOpsEx(system->global, "INI_GETSTR", 5, &_iniReadStr);
  	ASM_setCpuOpsEx(system->global, "INI_SETSTR", 3, &_iniSetStr);
  	ASM_setCpuOpsEx(system->global, "INI_WRITE", 1, &_iniWrite);
    
    
    #if defined(USE_SCRIPT_STACK)
        ASM_setCpuOpsEx(system->global, "EXECUTE", 1, &_scriptExecute);
    #endif
  	return;
}


//initialize any external libraries if necessary
int ASM_extLibInit(ASMSys *system){

  	ASM_initExtLibOps(system);
  	return 1;
}


//place holder free function
void ASM_freeExtLib(ASMSys *system){
	_iniClose(system);
  	return;
}

#endif//BASM_COMPILE_EXTLIBS
