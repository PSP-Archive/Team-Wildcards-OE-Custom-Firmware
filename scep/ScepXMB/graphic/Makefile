TARGET = scepGraphic
OBJS = graphic.o memory.o pspmath.o exports.o

BUILD_PRX = 1

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

PRX_EXPORTS = exports.exp

PSP_FW_VERSION = 340

INCDIR = 
CFLAGS = -Os -G0 -Wall -fno-strict-aliasing -fno-builtin-printf
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = 
LIBS= 
LDFLAGS = -mno-crt0 -nostartfiles

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak