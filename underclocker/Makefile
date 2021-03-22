TARGET = gamecpu
OBJS = main.o blit.o conf.o font.o

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

#USE_PSPSDK_LIBC = 1
USE_KERNEL_LIBC = 1
#USE_KERNEL_LIBS = 1

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti 
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS = -mno-crt0 -nostartfiles
LIBS = -lpsppower

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak