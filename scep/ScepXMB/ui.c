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
#include <pspsdk.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspdisplay.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <systemctrl_se.h>

#include "ctrl.h"
#include "text.h"
#include "config.h"
#include "util.h"
#include "import/pspdisplay.h"
#include "import/psppower.h"
#include "import/psprtc.h"
#include <pspsystimer.h>

#define min(x, y) (x > y? y: x)
#define max(x, y) (x < y? y: x)

typedef struct _border{
	short x0;
	short y0;
	short x1;
	short y1;
}Border;

static scxOption * scx;

static void * (* pspAlloc)(SceUID * blockID, const char * name, SceSize size, int flag);

static int (* pspFree)(SceUID blockID);

static int (* initFont)();

static void (* freeFont)();

static int (* initGraphic)();

static void (* freeGraphic)();

static void (* getBorder)(Border * tb, int x0, int y0, int x1, int y1);

static void (* drawLine)(u32 color, int x0, int y0, int x1, int y1);

static void (* drawCRect)(u32 color, int r, Border * tb, int clear);

static void (* drawMCRect)(u32 color, int r, Border * tb);

static void (* clearRect)(u32 color, Border * tb);

static void (* clearCRect)(u32 color, int r, Border * tb);

static void (* drawText)(const char * text, u32 color, int x, int y, Border * tb);

static void (* drawBText)(const char * text, u32 text_color, u32 bg_color, u32 bt_color, int x, int y, Border * tb);

static void (* drawScroll)(int now, int max, u32 bg_color, u32 fg_color, Border * tb);

static void (* drawLScroll)(int now, int max, u32 bg_color, u32 fg_color, Border * tb);

static int (* finder)();

static void (* capture)();

#define mod_Graphic "scepGraphic"
#define lib_Graphic "scepGraphicLibrary"
#define lib_Memory "scepMemoryLibrary"
#define path_Graphic "ms0:/SCEP/modules/scepGraphic.prx"
static SceUID modid_Graphic = -1;
#define mod_Filemgr "scepFilemgr"
#define lib_Filemgr "scepFilemgrService"
#define path_Filemgr "ms0:/SCEP/modules/scepFilemgr.prx"
#define mod_Usbhostfs "USBHostFS"
#define path_Usbhostfs "ms0:/SCEP/modules/usbhostfs.prx"
#define mod_Capture "scepCapture"
#define lib_Capture "scepCaptureService"
#define path_Capture "ms0:/SCEP/modules/scepCapture.prx"

static char * fixString(const char * src, char * des)
{
	if(strlen(src) > 20)
	{
		strcpy(des, src);
		memset(&des[17], '.', 3);
		des[20] = 0;
		return des;
	}
	return (char *)src;
}

static int initUI()
{
	modid_Graphic = loadstartModule(mod_Graphic, path_Graphic);
	pspAlloc = (void *)pspModuleExportHelper(mod_Graphic, lib_Memory, "pspAlloc", NULL);
	pspFree = (void *)pspModuleExportHelper(mod_Graphic, lib_Memory, "pspFree", NULL);
	initFont = (void *)pspModuleExportHelper(mod_Graphic, lib_Graphic, "initFont", NULL);
	freeFont = (void *)pspModuleExportHelper(mod_Graphic, lib_Graphic, "freeFont", NULL);
	initGraphic = (void *)pspModuleExportHelper(mod_Graphic, lib_Graphic, "initGraphic", NULL);
	freeGraphic = (void *)pspModuleExportHelper(mod_Graphic, lib_Graphic, "freeGraphic", NULL);
	getBorder = (void *)pspModuleExportHelper(mod_Graphic, lib_Graphic, "getBorder", NULL);
	drawLine = (void *)pspModuleExportHelper(mod_Graphic, lib_Graphic, "drawLine", NULL);
	drawCRect = (void *)pspModuleExportHelper(mod_Graphic, lib_Graphic, "drawCRect", NULL);
	drawMCRect = (void *)pspModuleExportHelper(mod_Graphic, lib_Graphic, "drawMCRect", NULL);
	clearRect = (void *)pspModuleExportHelper(mod_Graphic, lib_Graphic, "clearRect", NULL);
	clearCRect = (void *)pspModuleExportHelper(mod_Graphic, lib_Graphic, "clearCRect", NULL);
	drawText = (void *)pspModuleExportHelper(mod_Graphic, lib_Graphic, "drawText", NULL);
	drawBText = (void *)pspModuleExportHelper(mod_Graphic, lib_Graphic, "drawBText", NULL);
	drawScroll = (void *)pspModuleExportHelper(mod_Graphic, lib_Graphic, "drawScroll", NULL);
	drawLScroll = (void *)pspModuleExportHelper(mod_Graphic, lib_Graphic, "drawLScroll", NULL);
	if(!initGraphic())
		return 0;
	return 1;
}

static void freeUI()
{
	freeGraphic();
	stopunloadModule(mod_Graphic, modid_Graphic);
}

extern u32 drawMenu(const char ** item, int item_num, int * pos, int x, int width, int y, int height, u32 key, int init)
{
	int lpos = *pos, i;
	u32 press_key;
	Border tb[item_num];
	char s[512];
	if(!init)
	{
		for(i = 0; i < item_num; i ++)
		{
			drawBText(fixString(item[i], s), scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, x + width * i, y + height * i, &tb[i]);
		}
		drawCRect(scx->SD_COLOR, 5, &tb[*pos], 0);
	}
	while(1)
	{
		if(lpos != *pos)
		{
			drawBText(fixString(item[lpos], s), scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, x + width * lpos, y + height * lpos, &tb[lpos]);
			drawBText(fixString(item[*pos], s), scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, x + width * (*pos), y + height * (*pos), &tb[*pos]);
			drawCRect(scx->SD_COLOR, 5, &tb[*pos], 0);
			lpos = *pos;
		}
		press_key = ctrl_waitmask(key);
		switch(press_key)
		{
			case PSP_CTRL_LEFT:
			if(init == 2)
				return press_key;
			*pos -= 1;
			if(*pos <= -1)
				*pos = item_num - 1;
			break;
			case PSP_CTRL_UP:
			*pos -= 1;
			if(*pos <= -1)
			{
				if(init == 2)
				{
					*pos += 1;
					return 2;
				}
				else *pos = item_num - 1;
			}
			break;
			case PSP_CTRL_RIGHT:
			if(init == 2)
				return press_key;
			*pos += 1;
			if(*pos >= item_num)
				*pos = 0;
			break;
			case PSP_CTRL_DOWN:
			*pos += 1;
			if(*pos >= item_num)
			{
				if(init == 2)
				{
					*pos -= 1;
					return 1;
				}
				else *pos = 0;
			}
			break;
			case PSP_CTRL_CROSS:
				if(!scx->ox_swap)
				{
					ctrl_waitrelease();
					return 0;
				}
				return press_key;
				break;
			case PSP_CTRL_CIRCLE:
				if(scx->ox_swap)
				{
					ctrl_waitrelease();
					return 0;
				}
				return press_key;
				break;
			default :
				return press_key;
				break;
		}
	}
	return 0;
}

int my_tick_handler()
{
	static int div = 59;
	div++;
	if(div > 59)
	{
		div = 0;
		pspTime psptime;
		Border ttb;
		char s[64];
		sceRtcGetCurrentClockLocalTime(&psptime);
		sprintf(s, "%02u:%02u:%02u", psptime.hour, psptime.minutes, psptime.seconds);
		drawBText(s, scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, 3, 230, &ttb);
	}
	return -1;
}

static void uiInfo()
{
	Border tb, ttb, stb;
	unsigned short pw;
	getBorder(&tb, -40, 190, 70, 260);
	drawMCRect(scx->BG_COLOR, 20, &tb);
	getBorder(&stb, 5, 202, 25, 210);
	if(scePowerIsBatteryExist())
		pw = scePowerGetBatteryLifePercent();
	else pw = 0;
	drawLScroll(pw, 100, scx->BT_COLOR, scx->BY_COLOR, &stb);
	char s[16];
	sprintf(s, "%u%%", pw);
	drawText(s, scx->TX_COLOR, 30, 210, &ttb);
	if(!scePowerIsBatteryExist())
		drawText("X", scx->TX_COLOR, 10, 210, &ttb);
	else
	{
		if(scePowerIsBatteryCharging())
			drawText("C", scx->TX_COLOR, 10, 210, &ttb);
		if(scePowerIsLowBattery())
			drawText("L", scx->TX_COLOR, 10, 210, &ttb);
	}
	unsigned short cpu = scePowerGetCpuClockFrequency();
	unsigned short bus = scePowerGetBusClockFrequency();
	sprintf(s, "%u/%u", cpu, bus);
	drawBText(s, scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, 5, 250, &ttb);
}

extern int uiOsk(char * string, int max, int x0, int y0)
{
	const char * key[] =
	{
		"@.1", "ABC2", "DEF3", "GHI4", "JKL5", "MNO6", "PQRS7", "TUV8", "WXYZ9", "! ?", "_=0", "Enter"
	};
	const char * keyc[] =
	{
		"1.,@:;()",
		"abcABC2",
		"defDEF3",
		"ghiGHI4",
		"jklJKL5",
		"mnoMNO6",
		"pqrsPQRS7",
		"tuvTUV8",
		"wxyzWXYZ9",
		" !?*&<>[]{}",
		"0-~=_+#$^\\|",
		""
	};
	
	Border tmp;
	u32 keys;
	char tmps[128];
	char shows[21];
	char items[15][4];
	char * item[15];
	int head = 0, offset = 0, press = 0, rfstring = 1;
	int x = 0, lx = 1, y = 0, ly = 1;
	strcpy(tmps, string);
	int len = strlen(tmps);
	tmps[len] = ' ';
	tmps[len + 1] = 0;
	offset = len;
	tmps[max] = 0;
	
	getBorder(&tmp, x0, y0 - 10, x0 + 189, y0 + 220);
	clearRect(0, &tmp);
	getBorder(&tmp, x0, y0 - 10, x0 + 189, y0 + 200);
	drawCRect(scx->BG_COLOR, 20, &tmp, 1);
	
	getBorder(&tmp, x0 + 10, y0, x0 + 179, y0 + 120);
	drawCRect(scx->BT_COLOR, 20, &tmp, 0);
	
	getBorder(&tmp, x0 + 5, y0 + 160, x0 + 185, y0 + 190);
	drawCRect(scx->BT_COLOR, 20, &tmp, 0);
	
	int i, j;
	for(j = 0; j < 4; j++)
	{
		for(i = 0; i < 3; i++)
		{
			drawBText(key[i + j * 3], scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, x0 + 20 + i * 55, y0 + 20 + j * 30, &tmp);
		}
	}
	
	while(1)
	{
		if(rfstring)
		{
			getBorder(&tmp, x0 + 5, y0 + 135, x0 + 180, y0 + 155);
			clearRect(scx->BG_COLOR, &tmp);
			if(strlen(tmps) == max && tmps[strlen(tmps) - 1] != ' ')
				len = strlen(tmps);
			else len = strlen(tmps) - 1;
			tmps[max] = 0;
			head = max(offset - 19, 0);
			strncpy(shows, &tmps[head], 20);
			shows[20] = 0;
			drawBText(shows, scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, x0 + 12, y0 + 150, &tmp);
			getBorder(&tmp, tmp.x0 + (offset - head) * 8 + 2, tmp.y0 + 2, tmp.x0 + (offset - head) * 8 + 10, tmp.y1 - 2);
			drawCRect(scx->SD_COLOR, 5, &tmp, 0);
			rfstring = 0;
		}
		if(lx != x || ly != y)
		{
			drawBText(key[lx + ly * 3], scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, x0 + 20 + lx * 55, y0 + 20 + ly * 30, &tmp);
			drawBText(key[x + y * 3], scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, x0 + 20 + x * 55, y0 + 20 + y * 30, &tmp);
			drawCRect(scx->SD_COLOR, 5, &tmp, 0);
			lx = x;
			ly = y;
		}
		keys = ctrl_waitmask(KEYSETO);
		switch(keys)
		{
			case PSP_CTRL_LEFT:
				x--;
				if(x < 0)
					x = 2;
				break;
			case PSP_CTRL_RIGHT:
				x++;
				if(x > 2)
					x = 0;
				break;
			case PSP_CTRL_UP:
				y--;
				if(y < 0)
					y = 3;
				break;
			case PSP_CTRL_DOWN:
				y++;
				if(y > 3)
					y = 0;
				break;
			case PSP_CTRL_LTRIGGER:
				offset--;
				if(offset >= 0)
				{
					if(offset < head)
						head--;
					rfstring = 1;
				}
				else offset = 0;
				break;
			case PSP_CTRL_RTRIGGER:
				offset++;
				if(offset < strlen(tmps))
				{
					if(offset >= head + 20)
						head++;
					rfstring = 1;
				}
				else offset--;
				break;
			case PSP_CTRL_SQUARE:
				if(offset == len)
					offset--;
				i = offset;
				while(i < strlen(tmps))
				{
					tmps[i] = tmps[i + 1];
					i++;
				}
				if(strlen(tmps) == (max - 1))
					strcat(tmps, " ");
				rfstring = 1;
				break;
			case PSP_CTRL_CROSS:
				if(!scx->ox_swap)
				{
					ctrl_waitrelease();
					return 0;
				}
				press = 1;
				break;
			case PSP_CTRL_CIRCLE:
				if(scx->ox_swap)
				{
					ctrl_waitrelease();
					return 0;
				}
				press = 1;
				break;
		}
		if(press)
		{
			if(x == 2 && y == 3)
			{
				if(tmps[strlen(tmps) - 1] == ' ')
					tmps[strlen(tmps) - 1] = 0;
				strcpy(string, tmps);
				return 1;
			}
			for(i = 0; i < strlen(keyc[x + y * 3]); i++)
			{
				sprintf(items[i], "%c", keyc[x + y * 3][i]);
				item[i] = items[i];
			}
			int tpos = 0;
			keys = drawMenu((const char **)item, strlen(keyc[x + y * 3]), &tpos, x0 + 10, 16, y0 + 180, 0, KEYSET1, 0);
			getBorder(&tmp, x0 + 5, y0 + 160, x0 + 185, y0 + 190);
			clearRect(scx->BG_COLOR, &tmp);
			drawCRect(scx->BT_COLOR, 20, &tmp, 0);
			if(keys)
			{
				if(len < max)
				{
					i = strlen(tmps);
					while(i >= offset)
					{
						tmps[i + 1] = tmps[i];
						i--;
					}
				}
				tmps[offset] = keyc[x + y * 3][tpos];
				rfstring = 1;
			}
			press = 0;
		}
	}
	return 0;
}

static int uiBrightness()
{
	u32 key;
	int level, unk, done = 0;
	sceDisplayGetBrightness(&level, &unk);
	Border tb, ttb;
	getBorder(&tb, 181, 40, 230, 150);
	drawMCRect(scx->BG_COLOR, 20, &tb);
	drawBText(BRIGHTNESS_MIN, scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, 200, 60, &ttb);
	drawBText(BRIGHTNESS_MAX, scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, 200, 140, &ttb);
	Border stb;
	getBorder(&stb, 190, 50, 195, 140);
	while(!done)
	{
		clearRect(scx->BG_COLOR, &stb);
		drawScroll(level, 100, scx->LN_COLOR, scx->SD_COLOR, &stb);
		key = ctrl_waitmask(KEYSET0);
		switch(key)
		{
			case PSP_CTRL_UP:
			level -= 4;
			break;
			case PSP_CTRL_DOWN:
			level += 4;
			break;
			case PSP_CTRL_CIRCLE:
			case PSP_CTRL_CROSS:
			done = 1;
			break;
		}
		if(level < 0)
			level = 0;
		if(level > 99)
			level = 99;
		sceDisplaySetBrightness(level, 0);
	}
	clearCRect(scx->BG_COLOR, 20, &tb);
	return 0;
}

static int uiCpuspeed()
{
	u32 key;
	char s[16];
	int cpu, done = 0, change = 0;
	cpu = scePowerGetCpuClockFrequency();
	sprintf(s, "%d", cpu);
	Border tb, ttb, stb, ctb;
	getBorder(&tb, 181, 40, 230, 150);
	drawMCRect(scx->BG_COLOR, 20, &tb);
	drawBText(SPEED_MIN, scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, 200, 60, &ttb);
	drawBText(SPEED_MAX, scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, 200, 140, &ttb);
	drawBText(s, scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, 200, 100, &ttb);
	getBorder(&stb, 190, 50, 195, 140);
	while(!done)
	{
		clearRect(scx->BG_COLOR, &stb);
		drawScroll(cpu, 333, scx->LN_COLOR, scx->SD_COLOR, &stb);
		clearRect(scx->BG_COLOR, &ttb);
		sprintf(s, "%d", cpu);
		drawBText(s, scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, 200, 100, &ttb);
		key = ctrl_waitmask(KEYSET0);
		switch(key)
		{
			case PSP_CTRL_UP:
			cpu -= 10;
			if(cpu < 33)
				cpu = 33;
			break;
			case PSP_CTRL_DOWN:
			cpu += 10;
			if(cpu > 333)
				cpu = 333;
			break;
			case PSP_CTRL_CROSS:
				if(scx->ox_swap)
				{
					change = 1;
				}
				done = 1;
				break;
			case PSP_CTRL_CIRCLE:
				if(!scx->ox_swap)
				{
					change = 1;
				}
				done = 1;
				break;
		}
		if(change)
		{
			setCpuspeed(cpu);
			getBorder(&ctb, -40, 190, 70, 260);
			clearCRect(scx->BG_COLOR, 20, &ctb);
			uiInfo();
		}
	}
	clearCRect(scx->BG_COLOR, 20, &tb);
	return 0;
}

static int uiOeconfig()
{
	u32 key;
	int pos = 0, tpos = 0;
	Border tb, ttb;
	getBorder(&tb, 181, 40, 270, 170);
	drawMCRect(scx->BG_COLOR, 20, &tb);
	SEConfig config;
	sctrlSEGetConfig(&config);
	u8 set[6];
	set[0] = config.usenoumd;
	set[1] = config.gamekernel150;
	set[2] = config.skiplogo;
	set[3] = config.umdactivatedplaincheck;
	set[4] = config.executebootbin;
	set[5] = config.useisofsonumdinserted;
	while(1)
	{
		key = drawMenu(oeconfig_item, 6, &pos, 192, 0, 60, 20, KEYSET0, 0);
		if(key)
		{
			getBorder(&ttb, 281, 40 + pos * 20, 325, 80 + pos * 20);
			drawMCRect(scx->BG_COLOR, 20, &ttb);
			tpos = set[pos];
			key = drawMenu(yesno_item, 2, &tpos, 292, 0, 55 + pos * 20, 20, KEYSET0, 0);
			clearCRect(scx->BG_COLOR, 20, &ttb);
			if(key)
			{
				if(key & (PSP_CTRL_CIRCLE | PSP_CTRL_CROSS))
				{
					set[pos] = tpos;
					config.usenoumd = set[0];
					config.gamekernel150 = set[1];
					config.skiplogo = set[2];
					config.umdactivatedplaincheck = set[3];
					config.executebootbin = set[4];
					config.useisofsonumdinserted = set[5];
					sctrlSESetConfig(&config);
				}
			}
		}
		else break;
	}
	clearCRect(scx->BG_COLOR, 20, &tb);
	return 0;
}

static int uiThemeMgr()
{
	u32 key;
	int pos = 0, tpos = 0, offset = 0;
	Border tb, ttb;
	char theme[10];
	getBorder(&tb, 181, 40, 270, 170);
	drawMCRect(scx->BG_COLOR, 20, &tb);
	SceIoDirent ent;
	memset(ent, 0, sizeof(SceIoDirent));
	SceUID dfd = sceIoDopen("ms0:/PSP/THEME");
	int num = 0;
	while(sceIoDread(dfd, &ent) > 0 && dfd >= 0)
	{
		if(ent.d_name[0] == '.' && (ent.d_name[1] == '.' || ent.d_name[1] == 0))
		{
			continue;
		}
		num++;
		memset(ent, 0, sizeof(SceIoDirent));
	}
	sceIoDclose(dfd);
	
}

static int uiSysctrl()
{
	int pos = 0;
	u32 key;
	Border tb;
	getBorder(&tb, 81, 40, 170, 150);
	drawMCRect(scx->BG_COLOR, 20, &tb);
	while(1)
	{
		key = drawMenu(sysctrl_item, 5, &pos, 92, 0, 60, 20, KEYSET0, 0);
		if(key)
		{
			switch(pos)
			{
				case 0:
				uiBrightness();
				break;
				case 1:
				uiCpuspeed();
				break;
				case 2:
				uiOeconfig();
				break;
				case 3:
				break;
				case 4:
				break;
				case 5:
				uiThemeMgr();
				break;
			}
		}
		else break;
	}
	clearCRect(scx->BG_COLOR, 20, &tb);
	return 0;
}

static int uiPower()
{
	int pos = 0;
	u32 key;
	Border tb;
	getBorder(&tb, 81, 120, 150, 210);
	drawMCRect(scx->BG_COLOR, 20, &tb);
	key = drawMenu(power_item, 4, &pos, 92, 0, 140, 20, KEYSET0, 0);
	clearCRect(scx->BG_COLOR, 20, &tb);
	if(key)
	{
		switch(pos)
		{
			case 0:
			scePowerRequestSuspend();
			return 1;
			case 1:
			scePowerRequestStandby();
			return 1;
			case 2:
			scePowerReboot();
			return 1;
			case 3:
			break;
		}
	}
	return 0;
}

static SceSysTimerId timer;
static u32 sw_count = 0;

int stopwatch_handler(void)
{
	static u8 i = 0;
	i++;
	sw_count ++;
	if(i > 2)
	{
		i = 0;
		Border tmp;
		char times[64];
		getTimeString(times, sw_count);
		drawBText(times, scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, 225, 40, &tmp);
	}
	return -1;
}

static int uiStopwatch()
{
	Border tb, tmp;
	u32 key;
	int stop_count = 0;
	char times[64];
	getBorder(&tb, 91, 20, 460, 240);
	drawCRect(scx->BG_COLOR, 20, &tb, 0);
	timer = sceSTimerAlloc();
	int stop = 1;
	getTimeString(times, 0);
	drawBText(times, scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, 225, 40, &tmp);
	sceSTimerSetHandler(timer, 799999, stopwatch_handler, 0);
	drawLine(scx->LN_COLOR, 105, 45, 450, 45);
	while(1)
	{
		key = ctrl_waitmask(KEYSETW);
		switch(key)
		{
			case PSP_CTRL_LTRIGGER:
				stop_count = 0;
				stop = 1;
				sw_count = 0;
				sceSTimerStopCount(timer);
				sceSTimerResetCount(timer);
				getTimeString(times, 0);
				drawBText(times, scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, 225, 40, &tmp);
				getBorder(&tmp, 108, 50, 450, 230);
				clearRect(scx->BG_COLOR, &tmp);
				break;
			case PSP_CTRL_RTRIGGER:
				if(stop_count < 27)
				{
					getTimeString(times, sw_count);
					drawBText(times, scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, stop_count > 17? 340: (stop_count > 8? 225: 110), 65 + (stop_count % 9) * 20, &tmp);
					stop_count ++;
				}
				break;
			case PSP_CTRL_CROSS:
			case PSP_CTRL_CIRCLE:
				if((!scx->ox_swap && key == PSP_CTRL_CROSS) || (scx->ox_swap && key == PSP_CTRL_CIRCLE))
				{
					ctrl_waitrelease();
					sceSTimerStopCount(timer);
					sceSTimerFree(timer);
					clearRect(0, &tb);
					return 0;
				}
				else
				{
					if(stop)
					{
						sceSTimerStartCount(timer);
						stop = 0;
					}
					else 
					{
						sceSTimerStopCount(timer);
						sw_count = 0;
						stop = 1;
					}
				}
				break;
		}
	}
}

static int uiEwatch()
{
	int pos = 0;
	u32 key;
	Border tb;
	getBorder(&tb, 81, 60, 170, 110);
	drawMCRect(scx->BG_COLOR, 20, &tb);
	key = drawMenu(ewatch_item, 2, &pos, 92, 0, 80, 20, KEYSET0, 0);
	clearCRect(scx->BG_COLOR, 20, &tb);
	if(key)
	{
		switch(pos)
		{
			case 0:
			uiStopwatch();
			break;
			case 1:
			//scePowerRequestStandby();
			break;
		}
	}
	return 0;
}

extern int uiMain()
{
	if(!initUI())
		return 0;
	scx = getScxOption();
	SceSysTimerId timer;
	int pos = 0, init = 0, bar = 1;
	u32 key;
	Border tb;
	getBorder(&tb, -40, 10, 70, 150);
	while(1)
	{
		if(bar)
		{
			drawBText("SCEP-XMB 1.7alpha internal build182", scx->TX_COLOR, 0, scx->BG_COLOR, 190, 265, &tb);
			uiInfo();
			timer = sceSTimerAlloc();
			sceSTimerStartCount(timer);
			sceSTimerSetHandler(timer, 799999, my_tick_handler, 0);
			bar = 0;
		}
		if(!init)
		{
			getBorder(&tb, -40, 10, 70, 150);
			drawMCRect(scx->BG_COLOR, 20, &tb);
		}
		key = drawMenu(main_item, 6, &pos, 5, 0, 35, 20, KEYSET0, init);
		if(key)
		{
			init = 1;
			switch(pos)
			{
				case 0:
				sceSTimerStopCount(timer);
				sceSTimerFree(timer);
				clearCRect(scx->BG_COLOR, 20, &tb);
				SceUID modid3 = loadstartModule(mod_Capture, path_Capture);
				capture = (void *)pspModuleExportHelper(mod_Capture, lib_Capture, "capture", NULL);
				capture();
				stopunloadModule(mod_Capture, modid3);
				init = 0;
				bar = 1;
				break;
				case 1:
				uiSysctrl();
				break;
				case 2:
				clearCRect(scx->BG_COLOR, 20, &tb);
				SceUID modid1 = loadstartModule(mod_Usbhostfs, path_Usbhostfs);
				SceUID modid2 = loadstartModule(mod_Filemgr, path_Filemgr);
				if(modid1 >= 0 && modid2 >= 0)
				{
					finder = (void *)pspModuleExportHelper(mod_Filemgr, lib_Filemgr, "finder", NULL);
					finder();
				}
				stopunloadModule(mod_Filemgr, modid2);
				if (scx->iso)
				{
					sceSTimerStopCount(timer);
					sceSTimerFree(timer);
					freeUI();
					return 1;
				}
				else
					stopunloadModule(mod_Usbhostfs, modid1);
				init = 0;
				break;
				case 3:
				uiEwatch();
				break;
				case 4:
				break;
				case 5:
				if(uiPower())
				{
					sceSTimerStopCount(timer);
					sceSTimerFree(timer);
					freeUI();
					return 0;
				}
				break;
			}
		}
		else break;
	}
	sceSTimerStopCount(timer);
	sceSTimerFree(timer);
	clearCRect(scx->BG_COLOR, 20, &tb);
	freeUI();
	return 0;
}
