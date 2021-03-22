TARGET			= multi_cfw_installer
OBJS			= mp3player.o psp_dxar.o common.o main.o

INCDIR			=
LIBDIR			=
LDFLAGS			=
LIBS			= -lz -lpsppower -lmad -lpspaudiolib -lpspaudio
CFLAGS			= -O2 -G0 -Wall -DUSE_MS0
CXXFLAGS		= $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS			= $(CFLAGS) -c

EXTRA_TARGETS	= EBOOT.PBP
PSP_EBOOT_TITLE	= 3.40* Unpacker (with mp3 player)
PSP_EBOOT_ICON  = ../common/ICON0.PNG
PSPSDK			= $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

main.o: ../common/main.c
	$(CXX) $(CXXFLAGS) -c ../common/main.c -o main.o

psp_dxar.o: psp_dxar.c
	$(CXX) $(CXXFLAGS) -c psp_dxar.c -o psp_dxar.o

common.o: ../common/common.c
	$(CXX) $(CXXFLAGS) -c ../common/common.c -o common.o

mp3player.o: ../common/mp3player.c
	$(CXX) $(CXXFLAGS) -c ../common/mp3player.c -o mp3player.o
