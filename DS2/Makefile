#---------------------------------------------------------------------------------
.SUFFIXES:

#-----------------------------------------------------------------------------------
#Some project information
#-----------------------------------------------------------------------------------
ASMCORE 	:= 	../core

#NAME 		:= 	$(shell basename $(CURDIR))
NAME 		:=	../Binary/DS2/BAGASMI_DS2
TARGET		:=	$(NAME)
BUILD		:=	build

#set source and include directories
SOURCES		:=	src $(ASMCORE)
INCLUDES	:=	include


#DS2 Specific stuff=================================================================
DSTWOSDK :=/mnt/DSTwo/DS2SDK
LIBBAG :=$(DSTWOSDK)/../libBAG

LIBS 	:=$(DSTWOSDK)/lib/libds2b.a -lc -lm -lgcc
EXTLIBS :=$(LIBBAG)/lib/libbag.a $(DSTWOSDK)/lib/libds2a.a

CFLAGS += -std=gnu99 -mips32 -O3 -Wall -Wno-char-subscripts -mno-abicalls -fno-pic -fno-builtin \
	-fno-exceptions -ffunction-sections -mlong-calls \
	-fomit-frame-pointer -msoft-float -G 4 \
	-D___SYS_DS2_ -DBMP_MODULE -DPNG_MODULE -DJPG_MODULE -DCOMPILE_AUDIO

#compiler tools
CROSS :=/opt/mipsel-4.1.2-nopic/bin
CC 	=$(CROSS)/mipsel-linux-gcc
AR 	=$(CROSS)/mipsel-linux-ar rcsv
#LD 	=$(CROSS)/mipsel-linux-ld
LD 	=$(CC)
OBJCOPY	=$(CROSS)/mipsel-linux-objcopy
NM	=$(CROSS)/mipsel-linux-nm
OBJDUMP	=$(CROSS)/mipsel-linux-objdump

#ds2 base library paths
FS_DIR 		=$(DSTWOSDK)/libsrc/fs
CONSOLE_DIR	=$(DSTWOSDK)/libsrc/console
KEY_DIR		=$(DSTWOSDK)/libsrc/key
ZLIB_DIR	=$(DSTWOSDK)/libsrc/zlib
CORE_DIR	=$(DSTWOSDK)/libsrc/core

LIBINCLUDES :=-I$(DSTWOSDK)/include -I$(FS_DIR) -I$(CONSOLE_DIR) -I$(KEY_DIR) -I$(ZLIB_DIR) -I$(CORE_DIR) \
	-I$(LIBBAG)/libsrc/include


LINKS :=$(DSTWOSDK)/specs/link.xn
STARTS :=$(DSTWOSDK)/specs/start.S
STARTO :=start.o

#Back to normal now==================================================================

CFLAGS		+= $(LIBINCLUDES) $(INCLUDE)
CFLAGS 		+= -DPLATFORM_DS2 -DCONFIG_FILE="<config.h>"
CXXFLAGS	:= $(CFLAGS)
LDFLAGS		=$(LINKS)

#-----------------------------------------------------------------------------------
#Building rules - Super awesome recursive makefile calling!
#-----------------------------------------------------------------------------------

#if in project root directory
ifneq ($(BUILD),$(notdir $(CURDIR)))

#collect source files and set up paths
export OUTPUT	:=	$(CURDIR)/$(TARGET)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES 		:= 	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))

#generate o files from c files
export OFILES	:=	$(CFILES:.c=.o) $(CPPFILES:.cpp=.o) $(SFILES:.S=.o) $(STARTO)

#include directories
export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					-I$(CURDIR)/$(BUILD)


#if this is a C project, use C compiler
#otherwise use C++ compiler
#ifeq ($(strip $(CPPFILES)),)
#	export LD	:=	$(LD)
#else
#	export LD 	:= $(LD)
#endif

.PHONY: all $(BUILD) clean release test

all: $(BUILD)

#create build directory, change into it, then call make on this makefile again
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile


clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET)

#-----------------------------------------------------------------------------------
#Build Targets when not in root directory
#-----------------------------------------------------------------------------------
else

#collection dependencies
DEPENDS	:=	$(OFILES:.o=.d)

#build targets
$(OUTPUT).bin: $(OFILES)

$(STARTO):
	@$(CC) $(CFLAGS) -c $(STARTS) -o $@

%.o: %.c
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@

%.o: %.cpp
	@echo $(notdir $<)
	$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@

%.o: %.S
	@echo $(notdir $<)
	$(CC) $(CFLAGS) $(DEPSDIR)/$*.d -D_ASSEMBLER_ -D__ASSEMBLY__ -c $< -o $@

%.bin:
	@echo linking $(notdir $@)
	@$(LD)  -nostdlib -static -T $(LDFLAGS) -o $@ $(OFILES) $(EXTLIBS) $(LIBS)
	@$(OBJCOPY) -O binary $(OUTPUT).bin $(OUTPUT).bin
	$(DSTWOSDK)/tools/makeplug $(OUTPUT).bin $(OUTPUT).plg
	@rm $(OUTPUT).bin
endif


#-----------------------------------------------------------------------------------
#Clean and package project directory in tar file
#-----------------------------------------------------------------------------------
release: clean
	tar -zcvf "goldd5_$(shell basename $(CURDIR)).tar.gz" *
