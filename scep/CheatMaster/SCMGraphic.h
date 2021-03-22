/*	
	SCEP-CheatMaster 1.0
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

#ifndef _SCMGRAPHIC_H_
#define _SCMGRAPHIC_H_

#include "SCMTypes.h"

class ScmPoint
{
	public :
		u16 x;
		u16 y;
		void setX(u16 px) {x = px;}
		void setY(u16 py) {y = py;}
		void setXY(u16 px, u16 py) {setX(px); setY(py);}
		void addX(s16 px) {x += px;}
		void addY(s16 py) {y += py;}
		void addXY(s16 px, s16 py) {addX(px); addY(py);}
		void copy(ScmPoint &p) {setXY(p.x, p.y);}
		
		ScmPoint() {}
		ScmPoint(u16 px, u16 py) {setXY(px, py);}
		ScmPoint(ScmPoint &p) {copy(p);}
};

class ScmGraphic
{
	public :
		enum ScreenParam
		{
			SCREEN_WIDTH = 480,
			SCREEN_HEIGHT = 272
		};
		enum RectCorner
		{
			CORNER_LT = 0x0000F000,
			CORNER_RT = 0x00000F00,
			CORNER_LB = 0x000000F0,
			CORNER_RB = 0x0000000F
		};
		enum DrawCharError
		{
			ENDPAGE = 0x00,
			ENDLINE = 0x0F,
			NEWLINE = 0xF0,
			DRAWED = 0xFF
		};
		ScmPoint p0, p1, pen;
		u32 color_FG,
			color_BG,
			color_SW,
			color_LN,
			color_FW;
		u8 line_height;
		static bool IS_FLOAT(float y);
		static bool IS_ALPHA(u32 color);
		static u8 A(u32 color);
		static u8 B(u32 color);
		static u8 G(u32 color);
		static u8 R(u32 color);
		static u32 BGR(u32 color);
	
		ScmGraphic() {}
		
		int getPixelformat() {return pixelformat;}
		int getBuffersize() {return buffersize;}
		bool status() {return stat;}
		void refresh();
		void restore();
		
		bool init(SceUID id);
		void free();
		
		void * getPixel(ScmPoint &p);
		u32 & getPixelColor(ScmPoint &p, u32 &color);
		void putPixel(u32 color, ScmPoint &p);
		void clearPixel(ScmPoint &p);
		void drawLine();
		void drawRect(int r, bool clear, u32 flag);
		void clearRect(u32 color);
		DrawCharError drawChar(u32 ch, bool flag);
		int drawString(const char * txt, bool flag);
		int drawString(const wchar_t * txt, bool flag);
		void blur();
		
	private :
		SceUID bid;
		int bufferwidth,
			pixelformat,
			unk,
			buffersize;
		void * vram;
		void * vram_buf;
		u32 * fontp;
		s8 * font;
		bool stat;
		
		float inRect(ScmPoint & point, int r, u32 flag);
		float CXY(float r, float d);
		u32 alphaColor(u8 a, u32 color1, u32 color2);
};

#endif /*_SCMGRAPHIC_H_*/
