
#include <pspkernel.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <pspdisplay.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ctrl.h"

#include "../graphic/graphic.h"
#include "../config.h"

PSP_MODULE_INFO("scepCapture", 0x1000, 1, 0);
PSP_NO_CREATE_MAIN_THREAD();

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

static void captureScreen(Border * tb)
{
	int pw = tb->x1 - tb->x0 + 1;
	int ph = tb->y1 - tb->y0 + 1;
	sceIoMkdir("ms0:/PICTURE", 0777);
	BITMAPFILEHEADER h1;
	BITMAPINFOHEADER h2;
	const unsigned char bm[2] = {0x42, 0x4D};
	u8 buf[pw * 3];
	char filename[256];
	int x, y, sfd, ss_fcount = 0, unk, pixelformat, linesize;
	void * vram;
	
	sceDisplayGetFrameBuf(&vram, &linesize, &pixelformat, &unk);

	while(1)
	{
		++ ss_fcount;
		sprintf(filename, "%s/screenshot%03d.bmp", "ms0:/PICTURE", ss_fcount);
		sfd = sceIoOpen(filename, PSP_O_RDONLY, 0777);
		if(sfd >= 0)
			sceIoClose(sfd);
		else
			break;
	}

	sfd = sceIoOpen(filename, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if(sfd < 0)
		return;

	sceIoWrite(sfd, bm, 2);

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

	sceIoWrite(sfd, &h1, sizeof(BITMAPFILEHEADER));
	sceIoWrite(sfd, &h2, sizeof(BITMAPINFOHEADER));
	
	u16 color;
	u32 color32;
	u16 * vram16;
	u32 * vram32;
	int i;
	u8 r = 0, g = 0, b = 0;
	switch (pixelformat)
	{
		case PSP_DISPLAY_PIXEL_FORMAT_565:
			vram16 = (u16 *)vram;
			for(y = tb->y1; y >= tb->y0; y--)
			{
				for(i = 0, x = tb->x0; x <= tb->x1; x++, i+=3)
				{
					color = vram16[x + y * linesize];
					r = (color & 0x1f) << 3; 
					g = ((color >> 5) & 0x3f) << 2 ;
					b = ((color >> 11) & 0x1f) << 3 ;
					buf[i] = b; buf[i+1] = g; buf[i+2] = r;
				}
				sceIoWrite(sfd, buf, 3 * pw);
			}
			break;
		case PSP_DISPLAY_PIXEL_FORMAT_5551:
			vram16 = (u16 *)vram;
			for(y = tb->y1; y >= tb->y0; y--)
			{
				for(i = 0, x = tb->x0; x <= tb->x1; x++, i+=3)
				{
					color = vram16[x + y * linesize];
					r = (color & 0x1f) << 3; 
					g = ((color >> 5) & 0x1f) << 3 ;
					b = ((color >> 10) & 0x1f) << 3 ;
					buf[i] = b; buf[i+1] = g; buf[i+2] = r;
				}
				sceIoWrite(sfd, buf, 3 * pw);
			}
			break;
		case PSP_DISPLAY_PIXEL_FORMAT_4444:
			vram16 = (u16 *)vram;
			for(y = tb->y1; y >= tb->y0; y--)
			{
				for(i = 0, x = tb->x0; x <= tb->x1; x++, i+=3)
				{
					color = vram16[x + y * linesize];
					r = (color & 0xf) << 4; 
					g = ((color >> 4) & 0xf) << 4 ;
					b = ((color >> 8) & 0xf) << 4 ;
					buf[i] = b; buf[i+1] = g; buf[i+2] = r;
				}
				sceIoWrite(sfd, buf, 3 * pw);
			}
			break;
		case PSP_DISPLAY_PIXEL_FORMAT_8888:
			vram32 = (u32 *)vram;
			for(y = tb->y1; y >= tb->y0; y--)
			{
				for(i = 0, x = tb->x0; x <= tb->x1; x++, i+=3)
				{
					color32 = vram32[x + y * linesize];
					r = color32 & 0xff; 
					g = (color32 >> 8) & 0xff;
					b = (color32 >> 16) & 0xff;
					buf[i] = b; buf[i+1] = g; buf[i+2] = r;
				}
				sceIoWrite(sfd, buf, 3 * pw);
			}
			break;
	}
	sceIoClose(sfd);
}

extern void capture()
{
	scepOption * scx = getConfig();
	u32 key;
	int x0 = scx->x0, y0 = scx->y0, x1 = scx->x1, y1= scx->y1;
	Border tb;
	getBorder(&tb, 0, 0, 479, 271);
	clearRect(0, &tb);
	int x0_odd = 0, y0_odd = 0, x1_odd = 479, y1_odd = 271;
	int done = 0;
	while(!done)
	{
		if(x0 != x0_odd || x1 != x1_odd || y0 != y0_odd || y1 != y1_odd)
		{
			if (x0 > x0_odd)
			{
				getBorder(&tb, x0_odd, y0_odd, x0, y1);
				clearRect(scx->BG_COLOR, &tb);
			}
			else if (x0 < x0_odd)
			{
				getBorder(&tb, x0, y0, x0_odd, y1_odd);
				clearRect(0, &tb);
			}
			if (x1 > x1_odd)
			{
				getBorder(&tb, x1_odd, y0_odd, x1, y1);
				clearRect(0, &tb);
			}
			else if (x1 < x1_odd)
			{
				getBorder(&tb, x1, y0_odd, x1_odd, y1_odd);
				clearRect(scx->BG_COLOR, &tb);
			}
			if (y0 > y0_odd)
			{
				getBorder(&tb, x0_odd, y0_odd, x1, y0);
				clearRect(scx->BG_COLOR, &tb);
			}
			else if (y0 < y0_odd)
			{
				getBorder(&tb, x0, y0, x1_odd, y0_odd);
				clearRect(0, &tb);
			}
			if (y1 > y1_odd)
			{
				getBorder(&tb, x0_odd, y1_odd, x1, y1);
				clearRect(0, &tb);
			}
			else if (y1 < y1_odd)
			{
				getBorder(&tb, x0_odd, y1, x1_odd, y1_odd);
				clearRect(scx->BG_COLOR, &tb);
			}
			drawLine(scx->TX_COLOR, x0, y0, x0, y1);
			drawLine(scx->TX_COLOR, x0, y0, x1, y0);
			drawLine(scx->TX_COLOR, x1, y0, x1, y1);
			drawLine(scx->TX_COLOR, x0, y1, x1, y1);
			x0_odd = x0;
			x1_odd = x1;
			y0_odd = y0;
			y1_odd = y1;
		}
		key = ctrl_waitmask(PSP_CTRL_TRIANGLE | PSP_CTRL_LEFT | PSP_CTRL_RIGHT | PSP_CTRL_UP | PSP_CTRL_DOWN | 0x000110 | 0x000140 | 0x000180 | 0x000120 | 0x000210 | 0x000240 | 0x000280 | 0x000220 | PSP_CTRL_CROSS | PSP_CTRL_CIRCLE);
		switch(key)
		{
			case PSP_CTRL_TRIANGLE:
				x0 = 0; y0 = 0; x1 = 479; y1= 271;
				x0_odd = x0;
				x1_odd = x1;
				y0_odd = y0;
				y1_odd = y1;
				getBorder(&tb, 0, 0, 479, 271);
				clearRect(0, &tb);
				break;
			case PSP_CTRL_LEFT:
				if (x0 >= 4)
				{
					x0 -=4;
					x1 -=4;
				}
				break;
			case PSP_CTRL_RIGHT:
				if (x1 <= 475)
				{
					x0 +=4;
					x1 +=4;
				}
				break;
			case PSP_CTRL_UP:
				if (y0 >= 4)
				{
					y0 -=4;
					y1 -=4;
				}
				break;
			case PSP_CTRL_DOWN:
				if (y1 <= 267)
				{
					y0 +=4;
					y1 +=4;
				}
				break;
			case 0x000110:
				if (y0 >= 4)
				y0 -=4;
				break;
			case 0x000210:
				if (y0 < y1 - 4)
				y0 +=4;
				break;
			case 0x000140:
				if (y1 <= 267)
				y1 +=4;
				break;
			case 0x000240:
				if (y1 > y0 + 4)
				y1 -=4;
				break;
			case 0x000180:
				if (x0 >= 4)
				x0 -=4;
				break;
			case 0x000280:
				if (x0 < x1 - 4)
				x0 +=4;
				break;
			case 0x000120:
				if (x1 <= 475)
				x1 +=4;
				break;
			case 0x000220:
				if (x1 > x0 + 4)
				x1 -=4;
				break;
			case PSP_CTRL_CROSS:
			case PSP_CTRL_CIRCLE:
				if((!scx->ox_swap && key == PSP_CTRL_CROSS) || (scx->ox_swap && key == PSP_CTRL_CIRCLE))
				{
					getBorder(&tb, 0, 0, 479, 271);
					clearRect(0, &tb);
					ctrl_waitrelease();
				}
				else
				{
					scx->x0 = x0; scx->y0 = y0; scx->x1 = x1; scx->y1 = y1;
					getBorder(&tb, x0, y0, x1, y1);
					captureScreen(&tb);
					getBorder(&tb, 0, 0, 479, 271);
					clearRect(0, &tb);
				}
				done = 1;
				break;
		}
	}
}

int module_start (SceSize argc, void* argp) 
{
	return 0;
}
