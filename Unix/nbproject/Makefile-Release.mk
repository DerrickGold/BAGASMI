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
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/input.o \
	${OBJECTDIR}/src/video.o \
	${OBJECTDIR}/src/main.o \
	${OBJECTDIR}/_ext/761097586/data.o \
	${OBJECTDIR}/_ext/761097586/bagasmi.o \
	${OBJECTDIR}/_ext/761097586/execops.o


# C Compiler Flags
CFLAGS=-m32 -std=gnu99 -DPLATFORM_PC -DCONFIG_FILE="<config.h>"

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-Lsrc/usr/lib src/usr/lib/libSDL.a src/usr/lib/libSDLmain.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/unix

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/unix: src/usr/lib/libSDL.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/unix: src/usr/lib/libSDLmain.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/unix: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -lm -ldl -lpthread -lSDL -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/unix ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/src/input.o: src/input.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc/usr/include -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/input.o src/input.c

${OBJECTDIR}/src/video.o: src/video.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc/usr/include -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/video.o src/video.c

${OBJECTDIR}/src/main.o: src/main.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc/usr/include -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/main.o src/main.c

${OBJECTDIR}/_ext/761097586/data.o: ../core/data.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/761097586
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc/usr/include -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/761097586/data.o ../core/data.c

${OBJECTDIR}/_ext/761097586/bagasmi.o: ../core/bagasmi.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/761097586
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc/usr/include -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/761097586/bagasmi.o ../core/bagasmi.c

${OBJECTDIR}/_ext/761097586/execops.o: ../core/execops.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/761097586
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc/usr/include -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/761097586/execops.o ../core/execops.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/unix

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
