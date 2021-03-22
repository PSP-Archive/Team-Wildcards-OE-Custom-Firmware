release: all
	mv SystemControl150.prx rtc.prx

TARGET = SystemControl150
OBJS = main.o

BUILD_PRX=1
PRX_EXPORTS=exports.exp

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

#USE_PSPSDK_LIBC = 1
USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR =
LDFLAGS += -mno-crt0 -nostartfiles
LIBS +=

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
