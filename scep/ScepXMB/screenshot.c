/* 
 * Copyright (C) 2006 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include "screenshot.h"

#define CAPTURE_DIR "ms0:/PSP/PHOTO/Capture"

static int linesize;
static void * vram;

extern void screenshot_init()
{
	sceIoMkdir("ms0:/PSP", 0777);
	sceIoMkdir("ms0:/PSP/PHOTO", 0777);
	sceIoMkdir(CAPTURE_DIR, 0777);
}

static void dot8888(void * vram, int x, int y, unsigned char * r, unsigned char * g, unsigned char * b)
{
	u32 c32 = ((u32 *)vram)[x + y * linesize];
	* r = c32 & 0xFF;
	* g = (c32 >> 8) & 0xFF;
	* b = (c32 >> 16) & 0xFF;
}

static void dot565(void * vram, int x, int y, unsigned char * r, unsigned char * g, unsigned char * b)
{
	u16 color = ((u16 *)vram)[x + y * linesize];
	* r = (color & 0x1f) << 3;
	* g = ((color >> 5) & 0x3f) << 2;
	* b = ((color >> 11) & 0x1f) << 3;
}

static void dot5551(void * vram, int x, int y, unsigned char * r, unsigned char * g, unsigned char * b)
{
	u16 color = ((u16 *)vram)[x + y * linesize];
	* r = (color & 0x1f) << 3;
	* g = ((color >> 5) & 0x1f) << 3;
	* b = ((color >> 10) & 0x1f) << 3;
}

static void dot4444(void * vram, int x, int y, unsigned char * r, unsigned char * g, unsigned char * b)
{
	u16 color = ((u16 *)vram)[x + y * linesize];
	* r = (color & 0x0f) << 4;
	* g = ((color >> 4) & 0x0f) << 4;
	* b = ((color >> 8) & 0x0f) << 4;
}

typedef struct tagBITMAPFILEHEADER {
        unsigned long   bfSize;
        unsigned long   bfReserved;
        unsigned long   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	unsigned long	biSize;
	long	biWidth;
	long	biHeight;
	unsigned short	biPlanes;
	unsigned short	biBitCount;
	unsigned long	biCompression;
	unsigned long	biSizeImage;
	long	biXPelsPerMeter;
	long	biYPelsPerMeter;
	unsigned long	biClrUsed;
	unsigned long	biClrImportant;
} BITMAPINFOHEADER;

extern void Screenshot()
{
	BITMAPFILEHEADER h1;
	BITMAPINFOHEADER h2;
	const unsigned char bm[2] = {0x42, 0x4D};
	unsigned char buf[480 * 3];
	char filename[256];
	int x, y, unk, pixelformat, fd, pw, ph, ss_fcount = 0;
	void (*dotcb)(void * vram, int x, int y, unsigned char * r, unsigned char * g, unsigned char * b);

	sceDisplayGetMode(&unk, &pw, &ph);
	if(pw > 480)
		return;
	sceDisplayGetFrameBuf(&vram, &linesize, &pixelformat, &unk);

	switch(pixelformat)
	{
	case PSP_DISPLAY_PIXEL_FORMAT_565:
		dotcb = dot565;
		break;
	case PSP_DISPLAY_PIXEL_FORMAT_5551:
		dotcb = dot5551;
		break;
	case PSP_DISPLAY_PIXEL_FORMAT_4444:
		dotcb = dot4444;
		break;
	case PSP_DISPLAY_PIXEL_FORMAT_8888:
		dotcb = dot8888;
		break;
	default:
		return;
	}

	do {
		++ ss_fcount;
		sprintf(filename, "%s/screenshot%03d.bmp", CAPTURE_DIR, ss_fcount);
		fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);
		if(fd >= 0)
			sceIoClose(fd);
		else
			break;
	} while(1);

	fd = sceIoOpen(filename, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if(fd < 0)
		return;

	sceIoWrite(fd, bm, 2);

	h1.bfSize = 3 * pw * ph + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 2;
	h1.bfReserved = 0;
	h1.bfOffBits = 2 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	h2.biSize = sizeof(BITMAPINFOHEADER);
	h2.biPlanes = 1;
	h2.biBitCount = 24;
	h2.biCompression = 0;
	h2.biWidth = pw;
	h2.biHeight = ph;
	h2.biSizeImage = 3 * pw * ph;
	h2.biXPelsPerMeter = 0xEC4;
	h2.biYPelsPerMeter = 0xEC4;
	h2.biClrUsed = 0;
	h2.biClrImportant = 0;

	sceIoWrite(fd, &h1, sizeof(BITMAPFILEHEADER));
	sceIoWrite(fd, &h2, sizeof(BITMAPINFOHEADER));

	for(y = (ph-1); y >= 0; y--){
		int i;
		for(i = 0, x = 0; x < pw; x++)
		{
			dotcb(vram, x, y, &buf[i + 2], &buf[i + 1], &buf[i]);
			i += 3;
		}
		sceIoWrite(fd, buf, 3 * pw);
	}
	sceIoClose(fd);
}
