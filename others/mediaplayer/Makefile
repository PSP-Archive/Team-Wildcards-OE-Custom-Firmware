TARGET = mediaplayer
OBJS = aacplayer.o mp3player.o oggplayer.o main.o

INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = 
LDFLAGS =
LIBS = -lpsppower -lmad -lAac -lmikmod -lvorbisidec -lpspaudiolib -lpspaudio -lm

EXTRA_TARGETS = EBOOT.PBP
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

plugin: mediaplayer.elf
	psp-strip mediaplayer.elf -o EBOOT.PBP
