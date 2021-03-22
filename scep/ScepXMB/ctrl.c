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

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include "ctrl.h"

#include <stdio.h>
#include <string.h>

#define CTRL_REPEAT_TIME 0x40000
#define CTRL_REPEAT_INTERVAL 0x12000

static unsigned int last_btn = 0;
static unsigned int last_tick = 0;
static unsigned int repeat_flag = 0;

extern unsigned int ctrl_read()
{
	SceCtrlData ctl;
	sceCtrlPeekBufferPositive(&ctl, 1);

	if (ctl.Buttons == last_btn)
	{
		if (ctl.TimeStamp - last_tick < (repeat_flag ? CTRL_REPEAT_INTERVAL : CTRL_REPEAT_TIME)) return 0;
		repeat_flag = 1;
		last_tick = ctl.TimeStamp;
		return last_btn;
	}
	repeat_flag = 0;
	last_tick = ctl.TimeStamp;
	last_btn  = ctl.Buttons;
	return last_btn;
}

extern void ctrl_waitrelease()
{
	SceCtrlData ctl;
	do {
		sceCtrlPeekBufferPositive(&ctl, 1);
		sceKernelDelayThread(20000);
	} while ((ctl.Buttons & 0xF0FFFF) != 0);
	last_tick = ctl.TimeStamp;
	last_btn  = ctl.Buttons;
}

extern unsigned int ctrl_waitmask(unsigned int keymask)
{
	unsigned int key;
	while((key = (ctrl_read() & keymask)) == 0)
	{
		sceKernelDelayThread(20000);
	}
	return key;
}

extern unsigned int ctrl_waittime(unsigned int t)
{
	int i = 0, m = t * 50;
	unsigned int key;
	while((key = (ctrl_read() & 0xF0FFFF)) == 0 && i < m)
	{
		sceKernelDelayThread(20000);
		++ i;
	}
	return key;
}

extern unsigned int ctrl_input()
{
	ctrl_waitrelease();
	SceCtrlData ctl;
	unsigned int key, key2;
	do {
		sceCtrlPeekBufferPositive(&ctl, 1);
		key = ctl.Buttons & 0xF1FFFF;
	} while(key == 0);
	key2 = key;
	while((key2 & key) == key)
	{
		sceKernelDelayThread(20000);
		key = key2;
		sceCtrlPeekBufferPositive(&ctl, 1);
		key2 = ctl.Buttons & 0xF1FFFF;
	}
	ctrl_waitrelease();
	return key;
}

extern void get_keyname(unsigned int key, char * res)
{
	res[0] = 0;
	if((key & PSP_CTRL_CIRCLE) > 0)
		strcat(res, "O");
	if((key & PSP_CTRL_LTRIGGER) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "L");
	}
	if((key & PSP_CTRL_RTRIGGER) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "R");
	}
	if((key & PSP_CTRL_CROSS) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "X");
	}
	if((key & PSP_CTRL_SQUARE) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "[]");
	}
	if((key & PSP_CTRL_TRIANGLE) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "^");
	}
	if((key & PSP_CTRL_UP) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "UP");
	}
	if((key & PSP_CTRL_DOWN) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "DOWN");
	}
	if((key & PSP_CTRL_LEFT) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "LEFT");
	}
	if((key & PSP_CTRL_RIGHT) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "RIGHT");
	}
	if((key & PSP_CTRL_HOME) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "HOME");
	}
	if((key & PSP_CTRL_SELECT) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "SELECT");
	}
	if((key & PSP_CTRL_START) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "START");
	}
	if((key & PSP_CTRL_VOLUP) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "VOLUP");
	}
	if((key & PSP_CTRL_VOLDOWN) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "VOLDOWN");
	}
	if((key & PSP_CTRL_NOTE) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "NOTE");
	}
	if((key & PSP_CTRL_SCREEN) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "SCREEN");
	}
}

