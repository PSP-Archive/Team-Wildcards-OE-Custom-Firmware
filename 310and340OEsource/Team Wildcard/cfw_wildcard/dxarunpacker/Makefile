TARGET			= dxarunpacker
OBJS			= main.o psp_dxar.o common.o

INCDIR			=
LIBDIR			=
LDFLAGS			=
LIBS			= -lz -lpspvsh -lpsppower
CFLAGS			= -O2 -G0 -Wall
CXXFLAGS		= $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS			= $(CFLAGS) -c

EXTRA_TARGETS	= EBOOT.PBP
PSP_EBOOT_TITLE	= DXAR Unpacker
PSPSDK			= $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

main.o: main.c
	$(CXX) $(CXXFLAGS) -c main.c -o main.o

psp_dxar.o: psp_dxar.c
	$(CXX) $(CXXFLAGS) -c psp_dxar.c -o psp_dxar.o

common.o: common.c
	$(CXX) $(CXXFLAGS) -c common.c -o common.o
