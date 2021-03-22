TARGET = oeupdmaker
OBJS = main.o dxar.o

INCDIR = 
CFLAGS = -O2 -G0 -Wall -DBUILDER
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c

LIBDIR =
LDFLAGS = 
LIBS = -lpspsemaphore -lpspmesgd_driver -lz -lpsppower_driver

EXTRA_TARGETS = multi_cfw_installer.h EBOOT.PBP
PSP_EBOOT_TITLE = 3.40* All-In-One Update Maker/Unpacker
PSP_EBOOT_ICON = ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

multi_cfw_installer.h:
	cd flasher && make clean && make && cd ..
	cp flasher/multi_cfw_installer.h multi_cfw_installer.h
