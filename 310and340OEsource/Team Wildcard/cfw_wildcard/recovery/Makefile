TARGET = recovery
OBJS = main.o ctrl.o conf.o graphic.o menu.o time.o powerctrl.o registry.o util.o autoboot.o

BUILD_PRX = 1

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

PRX_EXPORTS = exports.exp

PSP_FW_VERSION = 150

INCDIR = 
CFLAGS = -Os -G0 -Wall -fno-strict-aliasing -fno-builtin-printf
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = 
LIBS= -lpsppower_driver  -lpspreg_driver -lpspusb_driver -lpspusbstor
LDFLAGS = -mno-crt0 -nostartfiles

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak