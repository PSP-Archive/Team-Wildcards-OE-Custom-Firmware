TARGET = cxmb
OBJS = main.o exports.o

BUILD_PRX = 1

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

PSP_FW_VERSION = 340

PRX_EXPORTS=exports.exp

INCDIR = 
CFLAGS = -Os -G0 -Wall -fno-strict-aliasing -fno-builtin-printf
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =  
LIBS = -lpsppower_driver -lpspreg_driver
LDFLAGS = -mno-crt0 -nostartfiles

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

