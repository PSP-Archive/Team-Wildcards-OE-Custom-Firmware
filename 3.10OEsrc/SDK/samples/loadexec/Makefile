all:
	mksfo 'LoadExec Test' PARAM.SFO
	pack-pbp EBOOT.PBP PARAM.SFO NULL NULL NULL NULL NULL loadexectest.prx NULL 

TARGET = loadexectest
OBJS = main.o

INCDIR = ../../include
CFLAGS = -O2 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
USE_PSPSDK_LIBC = 1

LIBDIR = ../../lib
LIBS = -lpspsystemctrl_user
LDFLAGS = -mno-crt0 -nostartfiles 

#EXTRA_TARGETS = EBOOT.PBP
#PSP_EBOOT_TITLE = LoadExec Test


PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
