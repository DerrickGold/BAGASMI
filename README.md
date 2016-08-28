#BAGASMI

Current Working Version: R7


    BAGASMI is a portable interpreter for an assembly like scripting language in which
one can bind their own functions to. The interpreter is written completely in C originally
for the Supercard DSTwo flashcard for the Nintendo DS. The Supercard DSTwo is a flashcard
which features its own onboard processor (120 to 396 mhz) and ram (32 megabytes).

For information on using this interpreter, take a look at the wiki!


==================================================================================
Compiling:
==================================================================================
-Enter the directory for the platform you wish to compile for
-modify makefile to reflect your system settings (change file locations and what-not)
-build using make
-std=gnu99 is required in your makefile to compile

Platform Notes:

DS2:
	-uses the standard Supercard SDK, requires libBAG(hasn't been released yet)
Can be compiled without libBAG by editing config.h and commenting out:

	#define BASM_COMPILE_VIDEO
	#define BASM_COMPILE_INPUT

for a text only binary

NDS:
	-compiles with the latest devkitARM and libnds

PC:
	-dev file can be opened with DevC++
	-requires SDL and compiles for windows
