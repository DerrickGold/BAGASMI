#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1853363519/bagasmi.o \
	${OBJECTDIR}/_ext/1853363519/execops.o \
	${OBJECTDIR}/_ext/1360937237/extLibs.o \
	${OBJECTDIR}/_ext/1360937237/input.o \
	${OBJECTDIR}/_ext/1360937237/video.o \
	${OBJECTDIR}/src/main.o


# C Compiler Flags
CFLAGS=-m32 -DPLATFORM_DS2 -DCONFIG_FILE="<config.h>" -std=gnu99 -DUSE_SDL -DJPG_MODULE -DBMP_MODULE -DPNG_MODULE

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../../../../libBAG/libsrcPC/linux/dist/Debug/GNU-Linux-x86/liblinux.a ../../../../libBAG/libsrcPC/linux/usr/lib/libSDL.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/ds2_simulator_linux

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/ds2_simulator_linux: ../../../../libBAG/libsrcPC/linux/dist/Debug/GNU-Linux-x86/liblinux.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/ds2_simulator_linux: ../../../../libBAG/libsrcPC/linux/usr/lib/libSDL.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/ds2_simulator_linux: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/ds2_simulator_linux ${OBJECTFILES} ${LDLIBSOPTIONS} -lm -ldl -lpthread

${OBJECTDIR}/_ext/1853363519/bagasmi.o: ../../core/bagasmi.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1853363519
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I../../../../libBAG/libsrc/include -I../../../../libBAG/libsrcPC/include -I../../../../libBAG/libsrcPC/linux/usr/include -I../../core -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1853363519/bagasmi.o ../../core/bagasmi.c

${OBJECTDIR}/_ext/1853363519/execops.o: ../../core/execops.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1853363519
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I../../../../libBAG/libsrc/include -I../../../../libBAG/libsrcPC/include -I../../../../libBAG/libsrcPC/linux/usr/include -I../../core -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1853363519/execops.o ../../core/execops.c

${OBJECTDIR}/_ext/1360937237/extLibs.o: ../src/extLibs.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I../../../../libBAG/libsrc/include -I../../../../libBAG/libsrcPC/include -I../../../../libBAG/libsrcPC/linux/usr/include -I../../core -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/extLibs.o ../src/extLibs.c

${OBJECTDIR}/_ext/1360937237/input.o: ../src/input.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I../../../../libBAG/libsrc/include -I../../../../libBAG/libsrcPC/include -I../../../../libBAG/libsrcPC/linux/usr/include -I../../core -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/input.o ../src/input.c

${OBJECTDIR}/_ext/1360937237/video.o: ../src/video.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I../../../../libBAG/libsrc/include -I../../../../libBAG/libsrcPC/include -I../../../../libBAG/libsrcPC/linux/usr/include -I../../core -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/video.o ../src/video.c

${OBJECTDIR}/src/main.o: src/main.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I../../../../libBAG/libsrc/include -I../../../../libBAG/libsrcPC/include -I../../../../libBAG/libsrcPC/linux/usr/include -I../../core -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/main.o src/main.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/ds2_simulator_linux

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
