TARGET = systemctrl
OBJS = main.o sysmodpatches.o isofs_driver.o umd9660_driver.o isoread.o csoread.o malloc.o conf.o kubridge.o systemctrl.o 

INCDIR = 
CFLAGS = -O2 -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1


LIBDIR =
LIBS = -lpspinit -lpspmodulemgr_kernel2 -lpspsysmem_kernel2  -lpsploadexec_kernel -lpspsemaphore
LDFLAGS =  -nostdlib -nodefaultlibs

PSPSDK=$(shell psp-config --pspsdk-path)
include build.mak

umd9660_driver.o:
	cp _umd9660_driver.o umd9660_driver.o