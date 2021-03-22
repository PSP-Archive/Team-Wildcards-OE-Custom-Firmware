/*	
	SCEP-XMB 1.1
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

#ifndef _CONFIG_H_
#define _CONFIG_H_

typedef struct _scep_option{
	u8 ox_swap;
	u8 iso;
	short x0;
	short y0;
	short x1;
	short y1;
	u32 BG_COLOR;
	u32 BT_COLOR;
	u32 TX_COLOR;
	u32 SD_COLOR;
	u32 LN_COLOR;
	u32 BK_COLOR;
	u32 BY_COLOR;
	char iso_path[256];
}scepOption;

typedef struct _scx_option{
	u8 ox_swap;
	u8 iso;
	u32 hotkey;
	u32 BG_COLOR;
	u32 BT_COLOR;
	u32 TX_COLOR;
	u32 SD_COLOR;
	u32 LN_COLOR;
	u32 BK_COLOR;
	u32 BY_COLOR;
	char iso_path[256];
}scxOption;

extern scepOption * getConfig();

extern scxOption * getScxOption();

extern scxOption * loadConfig();

extern int saveConfig();

#endif
