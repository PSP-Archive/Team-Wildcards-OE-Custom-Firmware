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

#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_

/** Structure store Border info */
typedef struct _border{
	short x0;
	short y0;
	short x1;
	short y1;
}Border;

/**
 * Initialize the graphic engine.
 *
 * @returns 1 on success, 0 on error.
*/
extern int initFont();

extern void freeFont();

extern int initGraphic();

/**
 * destroy the graphic engine.
*/
extern void freeGraphic();

extern void getBorder(Border * tb, int x0, int y0, int x1, int y1);

extern void drawLine(u32 color, int x0, int y0, int x1, int y1);

extern void drawCRect(u32 color, int r, Border * tb, int clear);

extern void drawMCRect(u32 color, int r, Border * tb);

extern void clearRect(u32 color, Border * tb);

extern void clearCRect(u32 color, int r, Border * tb);

extern void drawText(const char * text, u32 color, int x, int y, Border * tb);

extern void drawBText(const char * text, u32 text_color, u32 bg_color, u32 bt_color, int x, int y, Border * tb);

extern void drawScroll(int now, int max, u32 bg_color, u32 fg_color, Border * tb);

extern void drawLScroll(int now, int max, u32 bg_color, u32 fg_color, Border * tb);

#endif
