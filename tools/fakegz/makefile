OUTPUT=bin2fgz
OBJS=bin2fgz.o
LIBS=
CFLAGS=-Wall -I. -O2
LDFLAGS=-L.

ifdef BUILD_BIGENDIAN
CFLAGS += -DBUILD_BIGENDIAN
endif

ifdef BUILD_WIN32
CFLAGS  += -I../windows/lib -Wno-format
LDFLAGS += -L../windows/lib
OUTPUT := $(OUTPUT).exe
endif

install:
	make all
	cp -f $(OUTPUT) $(PSPDEV)/bin/$(OUTPUT)
	chmod a+x $(PSPDEV)/bin/$(OUTPUT)

all: $(OUTPUT)

clean:
	rm -f $(OUTPUT) *.o

$(OUTPUT): $(OBJS)
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS)
