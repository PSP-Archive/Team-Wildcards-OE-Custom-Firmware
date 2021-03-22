/*	
	SCEP-Series Graphic engine
    Copyright (C) 1997-2004 

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    
*/

#include <pspkernel.h>
#include <pspdisplay.h>

#include <stdio.h>
#include <string.h>

#include "graphic.h"
#include "memory.h"
#include "pspmath.h"

PSP_MODULE_INFO("scepGraphic", 0x1000, 1, 0);
PSP_NO_CREATE_MAIN_THREAD();

#define	PSP_LINE_SIZE 512
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define FRAMEBUFFER_SIZE (PSP_LINE_SIZE * SCREEN_HEIGHT)

#define IS_FLOAT(y) ((y - (int)y) > 0? 1: 0)
#define CXY(r, d) pspSqrt(pspSqr(r) - pspSqr(d))

#define IS_ALPHA(color) (((color) & 0xFF000000) == 0xFF000000? 0: 1)
#define A(color) ((u8)(color >> 24 & 0xFF))
#define B(color) ((u8)(color >> 16 & 0xFF))
#define G(color) ((u8)(color >> 8 & 0xFF))
#define R(color) ((u8)(color & 0xFF))
#define GCOLOR565(color) ((u32)ABGR_COLOR((((color >> 11) & 0x1F) << 3), (((color >> 5) & 0x3F) << 2), ((color & 0x1F) << 3)))
#define GCOLOR5551(color) ((u32)ABGR_COLOR(((color >> 10) & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, (color & 0x1F) << 3))
#define GCOLOR4444(color) ((u32)ABGR_COLOR(((color >> 8) & 0xF) << 4, ((color >> 4) & 0xF) << 4, (color & 0xF) << 4))
#define GCOLOR8888(color) ((u32)(color | 0xFF000000))
#define TCOLOR565(color) ((R(color) >> 3) | ((G(color) >> 2) << 5) | ((B(color) >> 3) << 11))
#define TCOLOR5551(color) ((R(color) >> 3) | ((G(color) >> 3) << 5) | ((B(color) >> 3) << 10))
#define TCOLOR4444(color) ((R(color) >> 4) | ((G(color) >> 4) << 4) | ((B(color) >> 4) << 8))
#define TCOLOR8888(color) (color & 0x00FFFFFF)
#define ABGR_COLOR(b, g, r) ((0xFF << 24) | (b << 16) | (g << 8) | r)
#define ACOLOR_SUB(a, c1, c2) ((u8)((a / 255.0f) * c1 + ((0xFF - a) / 255.0f) * c2))
#define ACOLOR(a, c1, c2) a? ABGR_COLOR(ACOLOR_SUB(a, B(c1), B(c2)), ACOLOR_SUB(a, G(c1), G(c2)), ACOLOR_SUB(a, R(c1), R(c2))): GCOLOR8888(c1)

typedef struct _color_24{
	u8 r;
	u8 g;
	u8 b;
}Color24;

typedef struct _font_info{
	u32 position;
	u8 width;
	u8 height;
	u8 left;
	u8 top;
}FontInfo;

static int bufferwidth, pixelformat, unk, Alpha_Channel = 1;
static void * vram;
static void * vram_buf;
static SceUID vb_id = -1, fb_id = -1, fnb_id = -1;
static FontInfo * font_info;
static u8 * font;

static void getColor(u32 * color, Color24 * color24)
{
	*color = ABGR_COLOR(color24->b, color24->g, color24->r);
}

static void getColor24(Color24 * color24, u32 * color)
{
	color24->r = R(*color);
	color24->g = G(*color);
	color24->b = B(*color);
}

extern int initFont()
{
	int fd = sceIoOpen("ms0:/SCEP/font.dat", PSP_O_RDONLY, 0777);
	if (fd < 0)
		return 0;
	font_info = (FontInfo *)pspAlloc(&fnb_id, "font_info", sizeof(FontInfo) * 95, 0);
	if (fnb_id < 0)
		return 0;
	sceIoRead(fd, font_info, sizeof(FontInfo) * 95);
	u32 size;
	sceIoRead(fd, &size, sizeof(u32));
	font = (u8 *)pspAlloc(&fb_id, "font", size, 0);
	if (fb_id < 0)
		return 0;
	sceIoRead(fd, font, size);
	sceIoClose(fd);
	return 1;
}

extern void freeFont()
{
	pspFree(fnb_id);
	pspFree(fb_id);
}

extern int initGraphic()
{
	if (!initFont())
		return 0;
	sceDisplayGetFrameBuf(&vram, &bufferwidth, &pixelformat, &unk);
	if(pixelformat == PSP_DISPLAY_PIXEL_FORMAT_8888)
	{
		vram_buf = pspAlloc(&vb_id, "screen_buffer", FRAMEBUFFER_SIZE * sizeof(Color24), 1);
		if (vb_id >= 0)
		{
			u32 * vram32 = (u32 *)vram;
			Color24 * vram32_buf = (Color24 *)vram_buf;
			int i;
			for(i = 0; i < FRAMEBUFFER_SIZE; i++)
				getColor24(&vram32_buf[i], &vram32[i]);
		}
		else Alpha_Channel = 0;
	}
	else
	{
		vram_buf = pspAlloc(&vb_id, "screen_buffer", FRAMEBUFFER_SIZE * 2, 1);
		if (vb_id < 0)
			memcpy(vram_buf, vram, FRAMEBUFFER_SIZE * 2);
		else Alpha_Channel = 0;
	}
	sceDisplaySetFrameBuf(vram, bufferwidth, pixelformat, 0);
	vram = (void*)(((u32)vram) | 0x40000000);
	return 1;
}

extern void freeGraphic()
{
	freeFont();
	pspFree(vb_id);
}

extern void getBorder(Border * tb, int x0, int y0, int x1, int y1)
{
	tb->x0 = x0;
	tb->x1 = x1;
	tb->y0 = y0;
	tb->y1 = y1;
}

static void * getPixelScreen(int x, int y)
{
	if(x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
		return NULL;
	if(pixelformat == PSP_DISPLAY_PIXEL_FORMAT_8888)
	{
		u32 * vram32 = (u32 *)vram;
		return &vram32[x + y * bufferwidth];
	}
	else
	{
		u16 * vram16 = (u16 *)vram;
		return &vram16[x + y * bufferwidth];
	}
}

static void putPixelScreen(u32 color, int x, int y)
{
	if(x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
		return;
	int a;
	if (IS_ALPHA(color))
	{
		a = A(color);
		if (a == 0)
			return;
	}
	else a = 0;
	if(pixelformat == PSP_DISPLAY_PIXEL_FORMAT_8888)
	{
		u32 * ncolor = (u32 *)getPixelScreen(x, y);
		*ncolor = TCOLOR8888(ACOLOR(a, color, GCOLOR8888(*ncolor)));
	}
	else
	{
		u16 * ncolor = (u16 *)getPixelScreen(x, y);
		switch (pixelformat)
		{
			case PSP_DISPLAY_PIXEL_FORMAT_565:
				*ncolor = TCOLOR565(ACOLOR(a, color, GCOLOR565(*ncolor)));
				break;
			case PSP_DISPLAY_PIXEL_FORMAT_5551:
				*ncolor = TCOLOR5551(ACOLOR(a, color, GCOLOR5551(*ncolor)));
				break;
			case PSP_DISPLAY_PIXEL_FORMAT_4444:
				*ncolor = TCOLOR4444(ACOLOR(a, color, GCOLOR4444(*ncolor)));
				break;
		}
	}
}

static void clearPixelScreen(int x, int y)
{
	if(x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
		return;
	if(pixelformat == PSP_DISPLAY_PIXEL_FORMAT_8888)
	{
		u32 * vram32 = (u32 *)vram;
		Color24 * vram32_buf = (Color24 *)vram_buf;
		getColor(&vram32[x + y * bufferwidth], &vram32_buf[x + y * bufferwidth]);
	}
	else
	{
		u16 * vram16 = (u16 *)vram;
		u16 * vram16_buf = (u16 *)vram_buf;
		vram16[x + y * bufferwidth] = vram16_buf[x + y * bufferwidth];
	}
}

static void wuPixelScreen(u32 color, float wx, float wy)
{
	int x = (int)wx;
	int y = (int)wy;
	float fx = wx - x;
	float fy = wy - y;
	putPixelScreen(((u8)(A(color) * (1 - fx) * (1 - fy)) << 24) | TCOLOR8888(color), x, y);
	if(fy)
		putPixelScreen(((u8)(A(color) * (1 - fx) * fy) << 24) | TCOLOR8888(color), x, y + 1);
	if(fx)
		putPixelScreen(((u8)(A(color) * fx * (1 - fy)) << 24) | TCOLOR8888(color), x + 1, y);
	if(fy && fx)
		putPixelScreen(((u8)(A(color) * fx * fy) << 24) | TCOLOR8888(color), x + 1, y + 1);
}

extern void drawLine(u32 color, int x0, int y0, int x1, int y1)
{
	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;
	
	if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;
	
	wuPixelScreen(color, x0, y0);
	if (dx > dy) {
		int fraction = dy - (dx >> 1);
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx;
			}
			x0 += stepx;
			fraction += dy;
			wuPixelScreen(color, x0, y0);
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			wuPixelScreen(color, x0, y0);
		}
	}
}

extern void drawCRect(u32 color, int r, Border * tb, int clear)
{
	u32 tcolor;
	int x, y, Cx, Cy, y0, y1;
	float Yn, Yh, tmp;
	for(x = tb->x0; x <= tb->x1; x ++)
	{
		if(x < (tb->x0 + r))
		{
			Cx = tb->x0 + r;
			Cy = tb->y0 + r;
			Yn = Cy - CXY(r, Cx - x);
			Yh = Cy - CXY(r, Cx - (x + 1));
			y0 = (int)Yn + IS_FLOAT(Yn);
			y1 = (int)(tb->y1 - r + CXY(r, Cx - x));
			while(Yn > Yh)
			{
				tmp = IS_FLOAT(Yn)? ((int)Yn + 1 - Yn): 0;
				tmp += ((((IS_FLOAT(Yn)? ((int)Yn + 1): Yn) - Yh) > 1? (x + 1 - Cx + CXY(r, Cy - (IS_FLOAT(Yn)? (int)Yn: (Yn - 1)))): 0) + (x + 1 - Cx + CXY(r, Cy - Yn))) * (((IS_FLOAT(Yn)? ((int)Yn + 1): Yn) - Yh) > 1? (1 - tmp): (Yn - Yh)) * 0.5f;
				tcolor = (((u8)(A(color) * tmp)) << 24) | TCOLOR8888(color);
				Yn = IS_FLOAT(Yn)? (int)Yn: (Yn - 1);
				if(clear) clearPixelScreen(x, Yn);
				putPixelScreen(tcolor, x, Yn);
			}
		}
		else if(x > (tb->x1 - r))
		{
			Cx = tb->x1 - r + 1;
			Cy = tb->y0 + r;
			Yh = Cy - CXY(r, x - Cx);
			Yn = Cy - CXY(r, x + 1 - Cx);
			y0 = (int)Yn + IS_FLOAT(Yn);
			y1 = (int)(tb->y1 - r + CXY(r, x + 1 - Cx));
			while(Yn > Yh)
			{
				tmp = IS_FLOAT(Yn)? ((int)Yn + 1 - Yn): 0;
				tmp += ((((IS_FLOAT(Yn)? ((int)Yn + 1): Yn) - Yh) > 1? (Cx + CXY(r, Cy - (IS_FLOAT(Yn)? (int)Yn: (Yn - 1))) - x): 0) + (Cx + CXY(r, Cy - Yn) - x)) * (((IS_FLOAT(Yn)? ((int)Yn + 1): Yn) - Yh) > 1? (1 - tmp): (Yn - Yh)) * 0.5f;
				tcolor = (((u8)(A(color) * tmp)) << 24) | TCOLOR8888(color);
				Yn = IS_FLOAT(Yn)? (int)Yn: (Yn - 1);
				if(clear) clearPixelScreen(x, Yn);
				putPixelScreen(tcolor, x, Yn);
			}
		}
		else
		{
			y0 = tb->y0;
			y1 = tb->y1;
		}
		for(y = y0; y <= y1; y ++)
		{
			if(clear) clearPixelScreen(x, y);
			putPixelScreen(color, x, y);
		}
		if(x < (tb->x0 + r))
		{
			Cx = tb->x0 + r;
			Cy = tb->y1 - r + 1;
			Yn = Cy + CXY(r, Cx - x);
			Yh = Cy + CXY(r, Cx - (x + 1));
			while(Yn < Yh)
			{
				tmp = IS_FLOAT(Yn)? (Yn - (int)Yn): 0;
				tmp += (((Yh - (int)Yn) > 1? (x + 1 - Cx + CXY(r, (int)Yn + 1 - Cy)): 0) + (x + 1 - Cx + CXY(r, Yn - Cy))) * ((Yh - (int)Yn) > 1? (1 - tmp): (Yh - Yn)) * 0.5f;
				tcolor = (((u8)(A(color) * tmp)) << 24) | TCOLOR8888(color);
				if(clear) clearPixelScreen(x, (int)Yn);
				putPixelScreen(tcolor, x, (int)Yn);
				Yn = (int)Yn + 1;
			}
		}
		else if(x > (tb->x1 - r))
		{
			Cx = tb->x1 - r + 1;
			Cy = tb->y1 - r + 1;
			Yh = Cy + CXY(r, x - Cx);
			Yn = Cy + CXY(r, x + 1 - Cx);
			while(Yn < Yh)
			{
				tmp = IS_FLOAT(Yn)? (Yn - (int)Yn): 0;
				tmp += (((Yh - (int)Yn) > 1? (Cx + CXY(r, (int)Yn + 1 - Cy) - x): 0) + (Cx + CXY(r, Yn - Cy) - x)) * ((Yh - (int)Yn) > 1? (1 - tmp): (Yh - Yn)) * 0.5f;
				tcolor = (((u8)(A(color) * tmp)) << 24) | TCOLOR8888(color);
				if(clear) clearPixelScreen(x, (int)Yn);
				putPixelScreen(tcolor, x, (int)Yn);
				Yn = (int)Yn + 1;
			}
		}
	}
}

extern void drawMCRect(u32 color, int r, Border * tb)
{
	int x, y, Cx, Cy, y0, y1, i;
	float Yn;
	tb->x0 -= 10; tb->x1 -= 10;
	u8 a = A(color)/10;
	color = TCOLOR8888(color);
	for(i = 0; i < 10; i++)
	{
		color = ((A(color) + a) << 24) | TCOLOR8888(color);
		for(x = tb->x0; x <= (tb->x0 + r + 1); x++)
		{
			for(y = tb->y0; y <= tb->y1; y++)
			{
				Cx = tb->x0 + r;
				Cy = tb->y0 + r;
				Yn = Cy - CXY(r, Cx - x);
				y0 = (int)Yn + IS_FLOAT(Yn);
				y1 = (int)(tb->y1 - r + CXY(r, Cx - x));
				if(x > (tb->x0 + 1) && y > y0 && y < y1)
					continue;
				clearPixelScreen(x, y);
			}
		}
		tb->x0 += 1; tb->x1 += 1;
		drawCRect(color, r, tb, 1);
	}
}

extern void clearRect(u32 color, Border * tb)
{
	int x, y;
	for(x = tb->x0; x <= tb->x1; x++)
	{
		for(y = tb->y0; y <= tb->y1; y++)
		{
			clearPixelScreen(x, y);
			if(color) putPixelScreen(color, x, y);
		}
	}
}

extern void clearCRect(u32 color, int r, Border * tb)
{
	int i;
	u8 a = A(color)/10;
	for(i = 0; i < 10; i++)
	{
		color = ((A(color) - a) << 24) | TCOLOR8888(color);
		drawCRect(color, r, tb, 1);
	}
}

extern void drawText(const char * text, u32 color, int x, int y, Border * tb)
{
	int n = 0;
	int Cx = x;
	int i,j;
	int grey;
	u32 pixel;
	
	getBorder(tb, x - 2, y - 2, x + 2, y + 2);
	
	while (text[n] > 0)
	{
		if(tb->y0 > (y - font_info[text[n] - 32].height - 2))
			tb->y0 = y - font_info[text[n] - 32].height - 2;
		for (i = 0; i < font_info[text[n] - 32].height; i++)
		{
			for(j = 0; j < font_info[text[n] - 32].width; j++)
			{
				grey = font[font_info[text[n] - 32].position + i * font_info[text[n] - 32].width + j];
				if(grey > 0)
					pixel = (((u8)(A(color) * (grey/255.0f))) << 24) | TCOLOR8888(color);
				else
		  			pixel = 0;
				if(pixel)
		  			putPixelScreen(pixel, Cx + font_info[text[n] - 32].left + j, y - font_info[text[n] - 32].top + i);
			}
		}
		Cx += 8;
		tb->x1 = Cx + 2;
		n++;
	}
}

extern void drawBText(const char * text, u32 text_color, u32 bg_color, u32 bt_color, int x, int y, Border * tb)
{
	int n = 0;
	getBorder(tb, x - 2, y - 2, x + 2, y + 2);
	while(text[n] > 0)
	{
		if(tb->y0 > (y - font_info[text[n] - 32].height - 2))
			tb->y0 = y - font_info[text[n] - 32].height - 2;
		tb->x1 += 8;
		n ++;
	}
	clearRect(bg_color, tb);
	drawCRect(bt_color, 5, tb, 0);
	drawText(text, text_color, x, y, tb);
}

extern void drawScroll(int now, int max, u32 bg_color, u32 fg_color, Border * tb)
{
	Border ttb;
	drawCRect(bg_color, 2, tb, 0);
	int y0 = now * (tb->y1 - tb->y0 - 8) / max + tb->y0;
	getBorder(&ttb, tb->x0, y0, tb->x1, y0 + 8);
	drawCRect(fg_color, 2, &ttb, 0);
}

extern void drawLScroll(int now, int max, u32 bg_color, u32 fg_color, Border * tb)
{
	Border ttb;
	drawCRect(bg_color, 2, tb, 0);
	int x1 = now * (tb->x1 - tb->x0) / max + tb->x0;
	getBorder(&ttb, tb->x0, tb->y0, x1, tb->y1);
	drawCRect(fg_color, 2, &ttb, 0);
}

int module_start (SceSize argc, void* argp) 
{
	return 0;
}
