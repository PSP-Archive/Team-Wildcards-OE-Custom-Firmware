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

#include <pspkernel.h>
#include <pspctrl.h>
#include <pspsdk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

static scxOption scx;
static scepOption scep;

extern scxOption * getScxOption()
{
	return &scx;
}

extern scepOption * getConfig(scepOption * sc)
{
	scep.ox_swap = scx.ox_swap;
	scep.BG_COLOR = scx.BG_COLOR;
	scep.BT_COLOR = scx.BT_COLOR;
	scep.TX_COLOR = scx.TX_COLOR;
	scep.SD_COLOR = scx.SD_COLOR;
	scep.LN_COLOR = scx.LN_COLOR;
	scep.BK_COLOR = scx.BK_COLOR;
	scep.BY_COLOR = scx.BY_COLOR;
	return &scep;
}

extern scxOption * loadConfig()
{
	memset(&scx, 0, sizeof(scxOption));
	memset(&scep, 0, sizeof(scepOption));
	scep.x1 = 479;
	scep.y1 = 271;
	scx.hotkey = PSP_CTRL_SELECT;
	scx.BG_COLOR = 0xA0EAA134;
	scx.BT_COLOR = 0x30F7E9D4;
	scx.TX_COLOR = 0xFFFFFFFF;
	scx.SD_COLOR = 0x50D4E5F7;
	scx.LN_COLOR = 0x40000000;
	scx.BK_COLOR = 0x60000000;
	scx.BY_COLOR = 0xA06049EF;
	int fd = sceIoOpen("ms0:/SCEP/config.scx", PSP_O_RDONLY, 0777);
	if (fd < 0)
		return &scx;
	sceIoRead(fd, &scx, sizeof(scxOption));
	sceIoClose(fd);
	return &scx;
}

extern int saveConfig()
{
	scx.iso = scep.iso;
	scep.iso = 0;
	strcpy(scx.iso_path, scep.iso_path);
	int fd = sceIoOpen("ms0:/SCEP/config.scx", PSP_O_CREAT | PSP_O_TRUNC | PSP_O_WRONLY, 0777);
	if (fd < 0)
		return 0;
	sceIoWrite(fd, &scx, sizeof(scxOption));
	sceIoClose(fd);
	return 1;
}
