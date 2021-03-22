#include "common.h"

// font
extern unsigned char msx[];

// variables
static int bufferwidth; static int pixelformat;
static void * vram;
int p = 0; int x = 0; int y = 0;
int pwidth; int pheight; int unk;

int checkPixelFormat (int format, int checktype) {
	// 16-bit check
	if (checktype == 1) {
		if ((format == PSP_DISPLAY_PIXEL_FORMAT_565) || (format == PSP_DISPLAY_PIXEL_FORMAT_5551) || (format == PSP_DISPLAY_PIXEL_FORMAT_4444)) return 1;
		return 0;
	}

	// 32-bit check
	if (checktype == 2) {
		if (pixelformat == PSP_DISPLAY_PIXEL_FORMAT_8888) return 1;
		return 0;
	}
	return 0;
}

int blit_string(int sx, int sy, const char *msg, int bg) {
	int ox = sx;

	sceDisplayGetMode(&unk, &pwidth, &pheight);
	sceDisplayGetFrameBuf(&vram, &bufferwidth, &pixelformat, &unk);
	vram = (void *)(((unsigned long)vram) | 0x40000000);

	if (bufferwidth == 0) return -1;

	if ((!checkPixelFormat(pixelformat, 1)) && (!checkPixelFormat(pixelformat, 2))) return -1;

	// convert pointer
	unsigned int * vramaddr = (unsigned int *)vram;

	for(x = 0; msg[x]; x++) {
		u8 code = msg[x] & 0x7f;
		for(y = 0; y < 8; y++) {
			int offset = (sy + y) * bufferwidth + ox;
			u8 pmap = msx[code * 8 + y];
			for(p = 0; p < 6; p++) {
				if(pmap & 0x80) {
					// white text
					if (checkPixelFormat(pixelformat, 2)) vramaddr[offset] = 0x00FFFFFF;
					else if (checkPixelFormat(pixelformat, 1)) vramaddr[offset] = 0xFFFF;
				}
				else if (bg == 1) {
					// black background
					if (checkPixelFormat(pixelformat, 2)) vramaddr[offset] = 0x00000000;
					else if (checkPixelFormat(pixelformat, 1)) vramaddr[offset] = 0x0000;
				}
				pmap <<= 1;
				offset++;
			}
		}
		ox += 6;
	}
	return x;
}
