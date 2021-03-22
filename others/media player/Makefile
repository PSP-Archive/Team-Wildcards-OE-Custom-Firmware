TARGET = mediaplayer
OBJS = oggplayer.o main.o

INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = 
LDFLAGS =
LIBS = -lpspaudiolib -lpspaudio -lvorbisidec -lm -lmad -lmikmod

EXTRA_TARGETS = EBOOT.PBP
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

plugin: mediaplayer.elf
	psp-strip mediaplayer.elf -o EBOOT.PBP
