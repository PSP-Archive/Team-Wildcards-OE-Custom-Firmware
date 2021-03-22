TARGET = scepXMB
OBJS = crt0_prx.o ctrl.o ui.o util.o config.o exports.o log.o #screenshot.o log.o
OBJS += import/scepImport.o

BUILD_PRX = 1

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

PRX_EXPORTS = exports.exp

PSP_FW_VERSION = 340

INCDIR = ./include
CFLAGS = -Os -G0 -Wall -fno-strict-aliasing -fno-builtin-printf
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = ./lib
LIBS= -lpsppower_driver -lpspusb_driver -lpspusbbus_driver -lpspsystemctrl_kernel
LDFLAGS = -mno-crt0 -nostartfiles

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak