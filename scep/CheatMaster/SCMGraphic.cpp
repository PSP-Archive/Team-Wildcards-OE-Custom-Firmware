/*	
	SCEP-Series Graphic engine
    Copyright (C) 2003-2007 

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

#include <pspdisplay.h>

#include "SCMGraphic.h"

inline float ScmGraphic::CXY(float r, float d)
{
	float result;
	asm("sqrt.s %0, %1": "=f"(result): "f"(r*r - d*d));
	return result;
}

inline u32 ScmGraphic::alphaColor(u8 a, u32 color1, u32 color2)
{
	return (u32)((u8)(R(color1) * a/255.0f + R(color2) * (255 - a)/255.0f) | ((u8)(G(color1) * a/255.0f + G(color2) * (255 - a)/255.0f) << 8) | ((u8)(B(color1) * a/255.0f + B(color2) * (255 - a)/255.0f) << 16));
}

void ScmGraphic::refresh()
{
	sceDisplayWaitVblankStart();
	sceDisplaySetFrameBuf((void *)((u32)vram & ~0x40000000), bufferwidth, pixelformat, 0);
}

void ScmGraphic::restore()
{
	if(pixelformat == PSP_DISPLAY_PIXEL_FORMAT_8888)
	{
		memcpy(vram, vram_buf, buffersize);
	}
	else
	{
		memcpy(vram, vram_buf, buffersize);
	}
}

bool ScmGraphic::init(SceUID id)
{
	bid = id;
	stat = false;
	color_BG = 0x80DCDAF1;
	color_FG = 0xC0000000;
	color_LN = 0x804D4D4D;
	color_FW = 0xFF000000;
	color_SW = 0x02909090;
	line_height = 15;
	sceDisplayGetFrameBuf(&vram, &bufferwidth, &pixelformat, &unk);
	vram = (void *)((u32)vram | 0x40000000);
	if(pixelformat == PSP_DISPLAY_PIXEL_FORMAT_8888)
	{
		buffersize = bufferwidth * SCREEN_HEIGHT * 4;
	}
	else
	{
		buffersize = bufferwidth * SCREEN_HEIGHT * 2;
	}
	vram_buf = sceKernelAllocHeapMemory(bid, buffersize);
	memcpy(vram_buf, vram, buffersize);
	
	//load font...
	int fd = sceIoOpen("ms0:/font.dat", PSP_O_RDONLY, 0777);
	if (fd < 0)
		return 0;
	fontp = (u32 *)sceKernelAllocHeapMemory(bid, 0xFFFF * sizeof(u32));
	sceIoRead(fd, fontp, 0xFFFF * sizeof(u32));
	u32 pos;
	sceIoRead(fd, &pos, sizeof(u32));
	font = (s8 *)sceKernelAllocHeapMemory(bid, pos);
	sceIoRead(fd, font, pos);
	sceIoClose(fd);
	
	refresh();
	pen.setXY(0, 0);
	stat = true;
	return 1;
}

void ScmGraphic::free()
{
	restore();
	refresh();
	sceKernelFreeHeapMemory(bid, vram_buf);
	sceKernelFreeHeapMemory(bid, fontp);	
	sceKernelFreeHeapMemory(bid, font);
	stat = false;
}

bool ScmGraphic::IS_FLOAT(float y)
{
	return (y - (int)y) > 0? true: false;
}

bool ScmGraphic::IS_ALPHA(u32 color)
{
	return ((color) & 0xFF000000) == 0xFF000000? false: true;
}

u8 ScmGraphic::A(u32 color)
{
	return (u8)(color >> 24 & 0xFF);
}

u8 ScmGraphic::B(u32 color)
{
	return (u8)(color >> 16 & 0xFF);
}

u8 ScmGraphic::G(u32 color)
{
	return (u8)(color >> 8 & 0xFF);
}

u8 ScmGraphic::R(u32 color)
{
	return (u8)(color & 0xFF);
}

u32 ScmGraphic::BGR(u32 color)
{
	return color & 0x00FFFFFF;
}

void * ScmGraphic::getPixel(ScmPoint &p)
{
	if(p.x >= SCREEN_WIDTH || p.y >= SCREEN_HEIGHT)
		return NULL;
	if(pixelformat == PSP_DISPLAY_PIXEL_FORMAT_8888)
	{
		return (void *)((u32)vram + (p.x + p.y * bufferwidth) * 4);
	}
	else
	{
		return (void *)((u32)vram + (p.x + p.y * bufferwidth) * 2);
	}
}

u32 & ScmGraphic::getPixelColor(ScmPoint &p, u32 &color)
{
	u16 &color16 = *((u16 *)getPixel(p));
	switch (pixelformat)
	{
		case PSP_DISPLAY_PIXEL_FORMAT_565:
			color = ((color16 & 0x1f) << 3) | (((color16 >> 5) & 0x3f) << 10) | (((color16 >> 11) & 0x1f) << 19);
			break;
		case PSP_DISPLAY_PIXEL_FORMAT_5551:
			color = ((color16 & 0x1f) << 3) | (((color16 >> 5) & 0x1f) << 11) | (((color16 >> 10) & 0x1f) << 19);
			break;
		case PSP_DISPLAY_PIXEL_FORMAT_4444:
			color = ((color16 & 0xf) << 4) | (((color16 >> 4) & 0xf) << 12) | (((color16 >> 8) & 0xf) << 20);
			break;
		case PSP_DISPLAY_PIXEL_FORMAT_8888:
			color = *((u32 *)getPixel(p));
			break;
	}
	return color;
}

void ScmGraphic::putPixel(u32 color, ScmPoint &p)
{
	if(p.x >= SCREEN_WIDTH || p.y >= SCREEN_HEIGHT)
		return;
	int a = 0;
	if (IS_ALPHA(color))
	{
		a = A(color);
		if (a == 0)
			return;
		u32 color_tmp;
		color = alphaColor(a, color, getPixelColor(p, color_tmp));
	}
	if(pixelformat == PSP_DISPLAY_PIXEL_FORMAT_8888)
	{
		*((u32 *)getPixel(p)) = color;
	}
	else
	{
		u16 color16 = 0;
		switch (pixelformat)
		{
			case PSP_DISPLAY_PIXEL_FORMAT_565:
				color16 = (R(color) >> 3) | ((G(color) >> 2) << 5) | ((B(color) >> 3) << 11);
				break;
			case PSP_DISPLAY_PIXEL_FORMAT_5551:
				color16 = (R(color) >> 3) | ((G(color) >> 3) << 5) | ((B(color) >> 3) << 10);
				break;
			case PSP_DISPLAY_PIXEL_FORMAT_4444:
				color16 = (R(color) >> 4) | ((G(color) >> 4) << 4) | ((B(color) >> 4) << 8);
				break;
		}
		*((u16 *)getPixel(p)) = color16;
	}
}

void ScmGraphic::clearPixel(ScmPoint &p)
{
	if(p.x >= SCREEN_WIDTH || p.y >= SCREEN_HEIGHT)
		return;
	if(pixelformat == PSP_DISPLAY_PIXEL_FORMAT_8888)
	{
		*((u32 *)getPixel(p)) = *((u32 *)((u32)vram_buf + (p.x + p.y * bufferwidth) * 4));
	}
	else
	{
		*((u16 *)getPixel(p)) = *((u16 *)((u32)vram_buf + (p.x + p.y * bufferwidth) * 2));
	}
}

void ScmGraphic::drawLine()
{
	int dy = p1.y - p0.y, dx = p1.x - p0.x, stepx, stepy;
	
	pen.copy(p0);
	
	if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;
	
	putPixel(color_LN, pen);
	if (dx > dy) {
		int fraction = dy - (dx >> 1);
		while (pen.x != p1.x) {
			if (fraction >= 0) {
				pen.y += stepy;
				fraction -= dx;
			}
			pen.x += stepx;
			fraction += dy;
			putPixel(color_LN, pen);
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (pen.y != p1.y) {
			if (fraction >= 0) {
				pen.x += stepx;
				fraction -= dy;
			}
			pen.y += stepy;
			fraction += dx;
			putPixel(color_LN, pen);
		}
	}
}

float ScmGraphic::inRect(ScmPoint & point, int r, u32 flag)
{
	if(point.x < p0.x || point.x > p1.x)
	{
		return 1;
	}
	else
	{
		float y0, y1;
		ScmPoint cc;
		if(point.x < (p0.x + r))
		{
			cc.setXY(p0.x + r, p0.y + r);
			y0 = (flag & CORNER_LT)? (cc.y - CXY(r, cc.x - point.x)): p0.y;
			cc.setXY(p0.x + r, p1.y - r);
			y1 = (flag & CORNER_LB)? (cc.y + CXY(r, cc.x - point.x)): p1.y;
		}
		else if(point.x > (p1.x - r))
		{
			cc.setXY(p1.x - r, p0.y + r);
			y0 = (flag & CORNER_RT)? (cc.y - CXY(r, point.x - cc.x)): p0.y;
			cc.setXY(p1.x - r, p1.y - r);
			y1 = (flag & CORNER_RB)? (cc.y + CXY(r, point.x - cc.x)): p1.y;
		}
		else
		{
			y0 = p0.y;
			y1 = p1.y;
		}
		if(point.y < y0)
		{
			return (y0 - point.y) > 1? 1: (y0 - point.y);
		}
		else if(point.y > y1)
		{
			return (point.y - y1) > 1? 1: (point.y - y1);
		}
	}
	return 0;
}

void ScmGraphic::drawRect(int r, bool clear, u32 flag)
{
	u32 tcolor;
	ScmPoint tpen, cc;
	int y0, y1, sw = A(flag);
	float Yn, Yh, tmp;
	for(pen.x = p0.x; pen.x <= p1.x; pen.x ++)
	{
		if(pen.x < (p0.x + r))
		{
			cc.setXY(p0.x + r, p0.y + r);
			Yn = cc.y - CXY(r, cc.x - pen.x);
			Yh = cc.y - CXY(r, cc.x - (pen.x + 1));
			y0 = (flag & CORNER_LT)? (int)Yn + IS_FLOAT(Yn) : p0.y;
			y1 = (flag & CORNER_LB)? (int)(p1.y - r + CXY(r, cc.x - pen.x)): p1.y;
			while(Yn > Yh && (flag & CORNER_LT))
			{
				tmp = IS_FLOAT(Yn)? ((int)Yn + 1 - Yn): 0;
				tmp += ((((IS_FLOAT(Yn)? ((int)Yn + 1): Yn) - Yh) > 1? (pen.x + 1 - cc.x + CXY(r, cc.y - (IS_FLOAT(Yn)? (int)Yn: (Yn - 1)))): 0) + (pen.x + 1 - cc.x + CXY(r, cc.y - Yn))) * (((IS_FLOAT(Yn)? ((int)Yn + 1): Yn) - Yh) > 1? (1 - tmp): (Yn - Yh)) * 0.5f;
				tcolor = (((u8)(A(color_BG) * tmp)) << 24) | BGR(color_BG);
				Yn = IS_FLOAT(Yn)? (int)Yn: (Yn - 1);
				tpen.setXY(pen.x, (u16)Yn);
				if(clear) clearPixel(tpen);
				putPixel(tcolor, tpen);
			}
		}
		else if(pen.x > (p1.x - r))
		{
			cc.setXY(p1.x - r + 1, p0.y + r);
			Yh = cc.y - CXY(r, pen.x - cc.x);
			Yn = cc.y - CXY(r, pen.x + 1 - cc.x);
			y0 = (flag & CORNER_RT)? (int)Yn + IS_FLOAT(Yn): p0.y;
			y1 = (flag & CORNER_RB)? (int)(p1.y - r + CXY(r, pen.x + 1 - cc.x)): p1.y;
			while(Yn > Yh  && (flag & CORNER_RT))
			{
				tmp = IS_FLOAT(Yn)? ((int)Yn + 1 - Yn): 0;
				tmp += ((((IS_FLOAT(Yn)? ((int)Yn + 1): Yn) - Yh) > 1? (cc.x + CXY(r, cc.y - (IS_FLOAT(Yn)? (int)Yn: (Yn - 1))) - pen.x): 0) + (cc.x + CXY(r, cc.y - Yn) - pen.x)) * (((IS_FLOAT(Yn)? ((int)Yn + 1): Yn) - Yh) > 1? (1 - tmp): (Yn - Yh)) * 0.5f;
				tcolor = (((u8)(A(color_BG) * tmp)) << 24) | BGR(color_BG);
				Yn = IS_FLOAT(Yn)? (int)Yn: (Yn - 1);
				tpen.setXY(pen.x, (u16)Yn);
				if(clear) clearPixel(tpen);
				putPixel(tcolor, tpen);
				if(sw && (pen.x + sw > p1.x))
				{
					for(int i = sw; i > 0; i --)
					{
						tpen.addXY(1, 1);
						putPixel(((((u8)(i * inRect(tpen, r, flag) * tmp * A(color_SW))) << 24) | BGR(color_SW)), tpen);
					}
				}
			}
		}
		else
		{
			y0 = p0.y;
			y1 = p1.y;
		}
		for(pen.y = y0; pen.y <= y1; pen.y ++)
		{
			if(clear) clearPixel(pen);
			putPixel(color_BG, pen);
			if((sw) && ((pen.y + sw > y1) || (pen.x + sw > p1.x)))
			{
				tpen.copy(pen);
				for(int i = sw; i > 0; i --)
				{
					tpen.addXY(1, 1);
					putPixel(((((u8)(i * inRect(tpen, r, flag) * A(color_SW))) << 24) | BGR(color_SW)), tpen);
				}
			}
		}
		if(pen.x < (p0.x + r) && (flag & CORNER_LB))
		{
			cc.setXY(p0.x + r, p1.y - r + 1);
			Yn = cc.y + CXY(r, cc.x - pen.x);
			Yh = cc.y + CXY(r, cc.x - (pen.x + 1));
			while(Yn < Yh)
			{
				tmp = IS_FLOAT(Yn)? (Yn - (int)Yn): 0;
				tmp += (((Yh - (int)Yn) > 1? (pen.x + 1 - cc.x + CXY(r, (int)Yn + 1 - cc.y)): 0) + (pen.x + 1 - cc.x + CXY(r, Yn - cc.y))) * ((Yh - (int)Yn) > 1? (1 - tmp): (Yh - Yn)) * 0.5f;
				tcolor = (((u8)(A(color_BG) * tmp)) << 24) | BGR(color_BG);
				tpen.setXY(pen.x, (int)Yn);
				if(clear) clearPixel(tpen);
				putPixel(tcolor, tpen);
				if(sw)
				{
					for(int i = sw; i > 0; i --)
					{
						tpen.addXY(1, 1);
						putPixel(((((u8)(i * inRect(tpen, r, flag) * tmp * A(color_SW))) << 24) | BGR(color_SW)), tpen);
					}
				}
				Yn = (int)Yn + 1;
			}
		}
		else if(pen.x > (p1.x - r) && (flag & CORNER_RB))
		{
			cc.setXY(p1.x - r + 1, p1.y - r + 1);
			Yh = cc.y + CXY(r, pen.x - cc.x);
			Yn = cc.y + CXY(r, pen.x + 1 - cc.x);
			while(Yn < Yh)
			{
				tmp = IS_FLOAT(Yn)? (Yn - (int)Yn): 0;
				tmp += (((Yh - (int)Yn) > 1? (cc.x + CXY(r, (int)Yn + 1 - cc.y) - pen.x): 0) + (cc.x + CXY(r, Yn - cc.y) - pen.x)) * ((Yh - (int)Yn) > 1? (1 - tmp): (Yh - Yn)) * 0.5f;
				tcolor = (((u8)(A(color_BG) * tmp)) << 24) | BGR(color_BG);
				tpen.setXY(pen.x, (int)Yn);
				if(clear) clearPixel(tpen);
				putPixel(tcolor, tpen);
				if(sw)
				{
					for(int i = sw; i > 0; i --)
					{
						tpen.addXY(1, 1);
						putPixel(((((u8)(i * inRect(tpen, r, flag) * tmp * A(color_SW))) << 24) | BGR(color_SW)), tpen);
					}
				}
				Yn = (int)Yn + 1;
			}
		}
	}
}

void ScmGraphic::clearRect(u32 color)
{
	for(pen.x = p0.x; pen.x <= p1.x; pen.x++)
	{
		for(pen.y = p0.y; pen.y <= p1.y; pen.y++)
		{
			clearPixel(pen);
			if(color) putPixel(color, pen);
		}
	}
}

ScmGraphic::DrawCharError ScmGraphic::drawChar(u32 ch, bool flag)
{
	if(pen.x + font[fontp[ch]] > p1.x)
	{
		return ENDLINE;
	}
	if(pen.y > p1.y)
	{
		return ENDPAGE;
	}
	if(ch == 0x0A || ch == 0x0D)
	{
		return NEWLINE;
	}
	
	ScmPoint tpen;
	u8 g[2];
	int wid, i, j, c;
	u8 gray;
	
	wid = font[fontp[ch] + 2] / 2 + ((font[fontp[ch] + 2] % 2)? 1: 0);
	for(i = 0; i < font[fontp[ch] + 1]; i++)
	{
		for(j = 0; j < font[fontp[ch] + 2]; j+=2)
		{
			gray = font[fontp[ch] + 5 + i * wid + j/2];
			g[0] = (gray & 0xF0) >> 4;
			g[1] = (gray & 0x0F);
			//g[2] = (gray & 0x0C) >> 2;
			//g[3] = (gray & 0x03);
			for(c = 0; c < 2 && (j + c) < font[fontp[ch] + 2]; c++)
			{
				if(g[c] > 0)
				{
					tpen.setXY(pen.x + font[fontp[ch] + 4] + j + c, pen.y - font[fontp[ch] + 3] + i);
					putPixel(((((u8)(g[c] * 17) * A(color_FG)/255) << 24) | BGR(color_FG)), tpen);
					for(int d = 3; d > 0 && flag; d --)
					{
						tpen.addXY(1, 1);
						putPixel(((((u8)(g[c] * d) * A(color_FW)/255) << 24) | BGR(color_FW)), tpen);
					}
				}
			}
		}
	}
	pen.x += font[fontp[ch]];
	return DRAWED;
}

int ScmGraphic::drawString(const char * txt, bool flag)
{
	ScmGraphic::DrawCharError error;
	pen.copy(p0);
	int n = 0, nl = 0;
	while(txt[n] > 0)
	{
		error = drawChar(txt[n], flag);
		if(error == ENDLINE || error == NEWLINE)
		{
			nl ++;
			pen.copy(p0);
			pen.addY(line_height * nl);
			if(error == NEWLINE)
			{
				n ++;
			}
		}
		else if(error == ENDPAGE)
		{
			return n;
		}
		else
		{
			n ++;
		}
	}
	return -1;
}

int ScmGraphic::drawString(const wchar_t * txt, bool flag)
{
	ScmGraphic::DrawCharError error;
	pen.copy(p0);
	int n = 0, nl = 0;
	while(txt[n] > 0)
	{
		error = drawChar(txt[n], flag);
		if(error == ENDLINE || error == NEWLINE)
		{
			nl ++;
			pen.copy(p0);
			pen.addY(line_height * nl);
			if(error == NEWLINE)
			{
				n ++;
			}
		}
		else if(error == ENDPAGE)
		{
			return n;
		}
		else
		{
			n ++;
		}
	}
	return -1;
}

void ScmGraphic::blur()
{
	int gauss_width = 7;
	int sumr = 0,
		sumg = 0,
		sumb = 0;
	int gauss_fact[] = {1, 6, 15, 20, 15, 6, 1};
	int gauss_sum = 64;
	u32 color;
	if(p0.x < 2)
	{
		p0.x = 2;
	}
	int width = p1.x - p0.x,
		height = p1.y - p0.y;
	int offset;
	u8 * r = (u8 *)sceKernelAllocHeapMemory(bid, width * height * sizeof(u8));
	u8 * g = (u8 *)sceKernelAllocHeapMemory(bid, width * height * sizeof(u8));
	u8 * b = (u8 *)sceKernelAllocHeapMemory(bid, width * height * sizeof(u8));
	for(int i = p0.x + 1 ; i < p1.x - 1; i ++)
	{
		for(int j = p0.y + 1; j < p1.y - 1; j ++)
		{
	    	sumr = sumg = sumb = 0;
	    	for(int k = 0; k < gauss_width; k ++)
			{
				pen.setXY(i - ((gauss_width - 1) >> 1) + k, j);
				color = getPixelColor(pen, color);
	      		sumr += R(color) * gauss_fact[k];
	      		sumg += G(color) * gauss_fact[k];
	      		sumb += B(color) * gauss_fact[k];
	    	}
			offset = width * (j - p0.y) + i - p0.x;
			r[offset] = sumr / gauss_sum;
			g[offset] = sumg / gauss_sum;
			b[offset] = sumb / gauss_sum;
		}
	}
	for(int i = p0.x + 1 ; i < p1.x - 1; i ++)
	{
		for(int j = p0.y + 3; j < p1.y - 3; j ++)
		{
	    	sumr = sumg = sumb = 0;
	    	for(int k = 0; k < gauss_width; k ++)
			{
				offset = width * ((j - p0.y) - ((gauss_width - 1) >> 1) + k) + i - p0.x;
	      		sumr += r[offset] * gauss_fact[k];
	      		sumg += g[offset] * gauss_fact[k];
	      		sumb += b[offset] * gauss_fact[k];
	    	}
			pen.setXY(i, j);
			putPixel(0xFF000000 | (u8)(sumb/gauss_sum) << 16 | (u8)(sumg/gauss_sum) << 8 | (u8)(sumr/gauss_sum), pen);
		}
	}
	sceKernelFreeHeapMemory(bid, r);
	sceKernelFreeHeapMemory(bid, g);	
	sceKernelFreeHeapMemory(bid, b);
}
