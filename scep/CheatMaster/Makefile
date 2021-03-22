#----------------------------------------------
#	Makefile for SCEP-CheatMaster 1.0
#
#	Copyright (C) 2003-2007            #----------------------------------------------

TARGET = SCEP-CheatMaster
BUILD_PRX = 1

#----------------------------------------------

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1
PSP_FW_VERSION = 340
PRX_EXPORTS = exports.exp

#----------------------------------------------

PSPSDK = $(shell psp-config --pspsdk-path)
SRCDIR = .
INCDIR = . $(PSPSDK)/include
LIBDIR = . $(PSPSDK)/lib

#----------------------------------------------

OBJS =  $(SRCDIR)/crt0_prx.o \
		$(SRCDIR)/SCMString.o \
		$(SRCDIR)/SCMCharset.o \
		$(SRCDIR)/SCMGraphic.o \
		$(SRCDIR)/SCMController.o \
		$(SRCDIR)/SCMDebugTools.o

#----------------------------------------------

EXPORT_OBJ = $(patsubst %.exp,%.o,$(PRX_EXPORTS))

#----------------------------------------------

#----------------------------------------------

CC       = psp-gcc
CXX      = psp-g++
AS       = psp-gcc
LD       = psp-gcc
AR       = psp-ar
RANLIB   = psp-ranlib
STRIP    = psp-strip
MKSFO    = mksfo
PACK_PBP = pack-pbp
FIXUP    = psp-fixup-imports

#----------------------------------------------

CFLAGS = $(addprefix -I,$(INCDIR)) \
			-Os -G0 -Wall -fno-strict-aliasing -fno-builtin-printf \
			-D_PSP_FW_VERSION=$(PSP_FW_VERSION)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS = $(addprefix -L,$(LIBDIR)) \
 			-specs=$(PSPSDK)/lib/prxspecs -Wl,-q,-T$(PSPSDK)/lib/linkfile.prx \
			-mno-crt0 -nostartfiles

#----------------------------------------------

ifeq ($(USE_KERNEL_LIBC),1)
# Use the PSP's kernel libc
PSPSDK_LIBC_LIB = 
CFLAGS := -I$(PSPSDK)/include/libc $(CFLAGS)
else
ifeq ($(USE_PSPSDK_LIBC),1)
# Use the pspsdk libc
PSPSDK_LIBC_LIB = -lpsplibc
CFLAGS := -I$(PSPSDK)/include/libc $(CFLAGS)
else
# Use newlib (urgh)
PSPSDK_LIBC_LIB = -lc
endif
endif

ifeq ($(USE_KERNEL_LIBS),1)
PSPSDK_LIBS = -lpspdebug -lpspdisplay_driver -lpspctrl_driver -lpspsdk
LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpspkernel
else
ifeq ($(USE_USER_LIBS),1)
PSPSDK_LIBS = -lpspdebug -lpspdisplay -lpspge -lpspctrl -lpspsdk
LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpspnet \
			-lpspnet_inet -lpspnet_apctl -lpspnet_resolver -lpsputility \
			-lpspuser
else
PSPSDK_LIBS = -lpspdebug -lpspdisplay -lpspge -lpspctrl -lpspsdk
LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpspnet \
			-lpspnet_inet -lpspnet_apctl -lpspnet_resolver -lpsputility \
			-lpspuser -lpspkernel
endif
endif

#----------------------------------------------

EXTRA_CLEAN = $(TARGET).elf $(EXPORT_OBJ)
FINAL_TARGET = $(TARGET).prx

#----------------------------------------------

all: $(FINAL_TARGET)

$(TARGET).elf: $(OBJS) $(EXPORT_OBJ)
	$(LINK.c) $^ $(LIBS) -o $@
	$(FIXUP) $@

%.prx: %.elf
	psp-prxgen $< $@

%.c: %.exp
	psp-build-exports -b $< > $@

clean: 
	-rm -f $(FINAL_TARGET) $(EXTRA_CLEAN) $(OBJS)

rebuild: clean all

#----------------------------------------------
