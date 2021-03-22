TARGET = popcorn
OBJS = main.o 

BUILD_PRX=1
PRX_EXPORTS=exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS=1

INCDIR = /usr/local/pspdev/psp/oesdk/include
CFLAGS = -O2 -G0 -Wall -fno-pic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = /usr/local/pspdev/psp/oesdk/lib
LIBS = -lpspinit -lpspsystemctrl_kernel -lpspsysmem_kernel2

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
