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
#include <pspusb.h>
#include <pspsysmem.h>
#include <usbhostfs.h>
#include <pspiofilemgr.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ctrl.h"
#include "../config.h"
#include "../ui.h"
#include "../graphic/memory.h"
#include "../graphic/graphic.h"

PSP_MODULE_INFO("scepFilemgr", 0x1000, 1, 0);
PSP_NO_CREATE_MAIN_THREAD();

#define min(x, y) (x > y? y: x)
#define max(x, y) (x < y? y: x)

typedef struct _dev_info
{
	u32 max_clusters;
	u32 free_clusters;
	u32 max_sectors;
	u32 sector_size;
	u32 sector_count;
}devInfo;

static const char * dev_item[] =
{
	"ms0:/",
	"host0:/",
	"disc0:/",
	"flash0:/",
	"flash1:/"
};

static const char * devn_item[] =
{
	"MS0",
	"USB",
	"UMD",
	"F0",
	"F1"
};

static const char * fop_item[] =
{
	"Copy",
	"Move",
	"Delete",
	"Rename",
	"Device",
	"Back"
};

static int buffer_size = 10*1024;
static u8 buffer[10*1024];
static scepOption * scx;

static void patch_Dev()
{
	u8 * addr = (u8 *)(0x88053A74);
	if (sceKernelDevkitVersion() == 0x03000310)
		addr = (u8 *)(0x88052854);
	if (sceKernelDevkitVersion() == 0x03010010)
		addr = (u8 *)(0x88053A74);
	int i;
	for(i = 0; i< 76; i++)
	{
		addr[i] = 0;
	}
}

static void startUSBHostFSDriver()
{
	patch_Dev();
	sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0); 
	sceUsbStart(HOSTFSDRIVER_NAME, 0, 0); 
	sceUsbActivate(HOSTFSDRIVER_PID);
}

static void stopUSBHostFSDriver()
{
	sceUsbDeactivate(HOSTFSDRIVER_PID);
	sceUsbStop(HOSTFSDRIVER_NAME, 0, 0);
	sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
}

static int filehandler(const char * base, const char * name)
{
	if(strstr(name, ".iso") || strstr(name, ".ISO") || strstr(name, ".cso") || strstr(name, ".CSO"))
	{
		strcpy(scx->iso_path, base);
		strcat(scx->iso_path, name);
		if(strstr(scx->iso_path, "host0:"))
		{
			int fd = sceIoOpen("ms0:/SCEP/usbhost", PSP_O_CREAT | PSP_O_TRUNC | PSP_O_WRONLY, 0777);
			if (fd < 0)
				return 0;
			sceIoWrite(fd, scx->iso_path, strlen(scx->iso_path)+1);
			sceIoClose(fd);
			scx->iso = 2;
			strcpy(scx->iso_path, "ms0:/ISO/monkey2.iso");
		}
		else if(strstr(scx->iso_path, "ms0:/ISO"))
		{
			
			scx->iso = 1;
		}
		saveConfig();
		return 1;
	}
	return 0;
}

static void getSpaceSize(char * sizes, u32 size)
{
	u32 sz, szd;
	if(size > 1024*1024*1024)
	{
		sz = size/(1024*1024*1024);
		szd = (size/(1024*1024) - sz * 1024) % 1024 /10;
		sprintf(sizes, "%u.%02uGB", sz, szd);
	}
	else if(size > 1024*1024)
	{
		sz = size/(1024*1024);
		szd = (size/1024 - sz * 1024) % 1024 /10;
		sprintf(sizes, "%u.%02uMB", sz, szd);
	}
	else if(size > 1024)
	{
		sz = size/1024;
		szd = (size - sz * 1024) % 1024 /10;
		sprintf(sizes, "%u.%02uKB", sz, szd);
	}
	else
	{
		sprintf(sizes, "%uByte", size);
	}
}

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

static void pspIoSpace(const char * devic, u32 * free_space, u32 * total_space)
{
	char dev[16];
	strcpy(dev, devic);
	if(dev[strlen(dev) - 1] == '/')
		dev[strlen(dev) - 1] = 0;
	devInfo devinfo;
	devInfo * info = &devinfo;
	if(!sceIoDevctl(devic, 0x02425818, &info, sizeof(info), NULL, 0))
	{
		*free_space = devinfo.free_clusters * devinfo.sector_count * devinfo.sector_size;
		*total_space = devinfo.max_clusters * devinfo.sector_count * devinfo.sector_size;
	}
	else
	{
		*free_space = 0;
		*total_space = 0;
	}
}

static int pspIoSeekEntry(SceUID dfd, int num)
{
	SceIoDirent ent;
	int i;
	memset(&ent, 0, sizeof(SceIoDirent));
	for(i = 0; i < num; i ++)
	{
		if(sceIoDread(dfd, &ent) <= 0)
			return 0;
		if(ent.d_name[0] == '.' && (ent.d_name[1] == '.' || ent.d_name[1] == 0))
		{
			i --;
		}
	}
	return 1;
}

static int pspIoRename(const char * ori, const char * new, const char * base)
{
	char orin[256];
	char newn[256];
	strcpy(orin, base);
	strcpy(newn, base);
	strcat(orin, ori);
	strcat(newn, new);
	if(sceIoRename(orin, newn) >= 0)
		return 1;
	else return 0;
}

static int pspIoCopyFile(const char * name, const char * src, const char * des)
{
	int fdsrc, fddes, bytes = 1;
	char path[256];
	strcpy(path, src);
	strcat(path, name);
	fdsrc = sceIoOpen(path, PSP_O_RDONLY, 0777);
	if(fdsrc < 0)
		return 0;
	strcpy(path, des);
	strcat(path, name);
	fddes = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if(fddes < 0)
		return 0;
	bytes = sceIoRead(fdsrc, buffer, buffer_size);
	while(bytes > 0)
	{
		sceIoWrite(fddes, buffer, bytes);
		bytes = sceIoRead(fdsrc, buffer, buffer_size);
	}
	sceIoClose(fddes);
	sceIoClose(fdsrc);
	return 1;
}

static int pspIoCopyDir(const char * name, const char * src, const char * des)
{
	int dfd, err;
	char src_path[256];
	char des_path[256];
	SceIoDirent ent;
	memset(&ent, 0, sizeof(SceIoDirent));
	strcpy(src_path, src);
	strcat(src_path, name);
	strcpy(des_path, des);
	strcat(des_path, name);
	sceIoMkdir(des_path, 0777);
	dfd = sceIoDopen(src_path);
	if(dfd < 0)
		return 0;
	strcat(src_path, "/");
	strcat(des_path, "/");
	while(sceIoDread(dfd, &ent) > 0)
	{
		if(ent.d_stat.st_attr & FIO_SO_IFDIR)
		{
			if(ent.d_name[0] == '.' && (ent.d_name[1] == '.' || ent.d_name[1] == 0))
				continue;
			else err = pspIoCopyDir(ent.d_name, src_path, des_path);
		}
		else err = pspIoCopyFile(ent.d_name, src_path, des_path);
	}
	sceIoDclose(dfd);
	return 1;
}

static int pspIoDeleteFile(const char * name, const char * base)
{
	char path[256];
	strcpy(path, base);
	strcat(path, name);
	sceIoRemove(path);
	return 1;
}

static int pspIoDeleteDir(const char * name, const char * base)
{
	int dfd, err;
	char path[256];
	SceIoDirent ent;
	memset(&ent, 0, sizeof(SceIoDirent));
	strcpy(path, base);
	strcat(path, name);
	
	dfd = sceIoDopen(path);
	strcat(path, "/");
	if(dfd < 0)
		return 0;
	while(sceIoDread(dfd, &ent) > 0)
	{
		if(ent.d_stat.st_attr & FIO_SO_IFDIR)
		{
			if(ent.d_name[0] == '.' && (ent.d_name[1] == '.' || ent.d_name[1] == 0))
				continue;
			else err = pspIoDeleteDir(ent.d_name, path);
		}
		else err = pspIoDeleteFile(ent.d_name, path);
	}
	sceIoDclose(dfd);
	path[strlen(path) - 1] = 0;
	sceIoRmdir(path);
	return 1;
}

static int done = 0;

int ioThread(SceSize args, void *argp)
{
	int err = 0;
	u32 * arg = (u32 *)argp;
	SceIoDirent * ent = (SceIoDirent *)arg[0];
	char * src = (char *)arg[1];
	char * des = (char *)arg[2];
	if(ent->d_stat.st_attr & FIO_SO_IFDIR)
	{
		if(args & 0x0F)
			err = pspIoCopyDir(ent->d_name, src, des);
		if(args & 0xF0)
			err = pspIoDeleteDir(ent->d_name, src);
	}
	else
	{
		if(args & 0x0F)
			err = pspIoCopyFile(ent->d_name, src, des);
		if(args & 0xF0)
			err = pspIoDeleteFile(ent->d_name, src);
	}
	done = 1;
	return err;
}

static int pspIoOperation(SceIoDirent * ent, const char * src, const char * des, u8 op)
{
	u32 argp[3];
	argp[0] = (u32)ent;
	argp[1] = (u32)src;
	argp[2] = (u32)des;
	SceUID tid = sceKernelCreateThread("scep_io_thread", ioThread, 48, 0x1000, 0x00100001 | PSP_THREAD_ATTR_CLEAR_STACK, NULL);
	done = 0;
	if(tid >= 0)
		sceKernelStartThread(tid, op, argp);
	while(!done)
		sceKernelDelayThread(100000);
	sceKernelTerminateDeleteThread(tid);
	return 0;
}

extern int finder()
{
	scx = getConfig();
	
	startUSBHostFSDriver();
	
	int offset[2], head[2], end[2];
	memset(offset, 0, 2 * sizeof(int));
	memset(head, 0, 2 * sizeof(int));
	memset(end, 0, 2 * sizeof(int));
	int cover = 1, neg = 1, reload = 1, update_entry = 1, i, num = 0, refresh = 4, tpos = 0, done = 0, newhead;
	u8 item_stat[150];
	int dev[2];
	dev[0] = 0; dev[1] = 0;
	memset(item_stat, 0, 150 * sizeof(u8));
	char * item[10];
	u32 key;
	SceUID dfd;
	SceIoDirent ent[10];
	char path[2][512];
	char s[512];
	strcpy(path[0], dev_item[dev[0]]);
	strcpy(path[1], dev_item[dev[1]]);
	
	Border tb[3];
	Border ttb[10];
	Border tmp;
	
	while(!done)
	{
		if(reload)
		{
			memset(ent, 0, 10 * sizeof(SceIoDirent));
			dfd = sceIoDopen(path[1 - neg]);
			num = 0;
			while(sceIoDread(dfd, &ent[9]) > 0 && dfd >= 0)
			{
				if(ent[9].d_name[0] == '.' && (ent[9].d_name[1] == '.' || ent[9].d_name[1] == 0))
				{
					continue;
				}
				num++;
			}
			sceIoDclose(dfd);
			reload = 0;
			update_entry = 1;
		}
		if(update_entry)
		{
			dfd = sceIoDopen(path[1 - neg]);
			pspIoSeekEntry(dfd, head[1 - neg]);
			end[1 - neg] = min(head[1 - neg] + 9, num);
			for(i = 0; i < (end[1 - neg] - head[1 - neg]); i++)
			{
				sceIoDread(dfd, &ent[i]);
				if(ent[i].d_name[0] == '.' && (ent[i].d_name[1] == '.' || ent[i].d_name[1] == 0))
					i--;
				else if(ent[i].d_stat.st_attr & FIO_SO_IFDIR)
					strcat(ent[i].d_name, "/");
			}
			sceIoDclose(dfd);
			update_entry = 0;
		}
		if(refresh == 1)
		{
			int pos = 0;
			getBorder(&tb[2], -40, 10, 70, 150);
			drawMCRect(scx->BG_COLOR, 20, &tb[2]);
			key = drawMenu(fop_item, 6, &pos, 5, 0, 35, 20, KEYSET0, 0);
			clearCRect(scx->BG_COLOR, 20, &tb[2]);
			if(key)
			{
				int tmp1;
				switch(pos)
				{
					case 0:
					case 1:
					case 2:
					freeFont();
					tmp1 = 0;
					for(i = 0; i < num; i++)
					{
						if(item_stat[i])
						{
							dfd = sceIoDopen(path[1 - neg]);
							pspIoSeekEntry(dfd, i - tmp1);
							sceIoDread(dfd, &ent[9]);
							while(ent[9].d_name[0] == '.' && (ent[9].d_name[1] == '.' || ent[9].d_name[1] == 0))
								sceIoDread(dfd, &ent[9]);
							sceIoDclose(dfd);
							pspIoOperation(&ent[9], path[1 - neg], path[neg], (!pos)? 0x0F:(pos < 2? 0xFF: 0xF0));
							if(pos) tmp1 ++;
						}
					}
					initFont();
					break;
					case 3:
					{
						char name[64];
						strcpy(name, ent[offset[1 - neg] - head[1 - neg]].d_name);
						if(ent[offset[1 - neg] - head[1 - neg]].d_stat.st_attr & FIO_SO_IFDIR)
							name[strlen(name) - 1] = 0;
						char oname[64];
						strcpy(oname, name);
						uiOsk(name, 64, neg? 281: 81, 20);
						pspIoRename(oname, name, path[1 - neg]);
					}
					break;
					case 4:
					key = drawMenu(devn_item, 5, &dev[1 - neg], neg? 92: 292, 35, 30, 0, KEYSET1, 0);
					if(key)
					{
						strcpy(path[1 - neg], dev_item[dev[1 - neg]]);
					}
					break;
					case 5:
					getBorder(&tmp, tb[0].x0, tb[0].y0, tb[1].x1, tb[1].y1 + 22);
					clearRect(0, &tmp);
					return 0;
				}
				reload = 1;
				if(pos == 0 || pos == 1)
				{
					neg = 1 - neg;
					cover = 1;
				}
				if(pos == 3) cover = 1;
				offset[1 - neg] = 0;
				head[1 - neg] = 0;
				memset(item_stat, 0, 150 * sizeof(u8));
				refresh = pos == 3? 4: (3 - neg);
				continue;
			}
		}
		if(refresh > 1)
		{
			int nref = (refresh == 4? 2: refresh), mref = (refresh == 2)? 3: 4;
			while(nref < mref)
			{
				getBorder(&tb[nref - 2], nref == 2? 81: 281, 10, nref == 2? 270: 470, 240);
				drawCRect(scx->BG_COLOR, 20, &tb[nref - 2], 1);
				for(i = 0; i < 5; i ++)
					drawBText(devn_item[i], scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, (nref == 2? 92: 292) + i * 35, 30, &ttb[i]);
				drawCRect(scx->SD_COLOR, 5, &ttb[dev[1 - neg]], 0);
				drawLine(scx->LN_COLOR, nref == 2? 92: 292, 35, nref == 2? 260: 460, 35);
				end[nref - 2] = min(head[nref - 2] + 9, num);
				for(i = head[nref - 2]; i < end[nref - 2]; i ++)
				{
					drawBText(fixString(ent[i - head[nref - 2]].d_name, s), scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, nref == 2? 92: 292, 50 + 20 * (i - head[nref - 2]), &ttb[i - head[nref - 2]]);
					if(item_stat[i])
						drawBText("*", scx->TX_COLOR, scx->BG_COLOR, scx->BY_COLOR, ttb[i - head[nref - 2]].x1 + 6, 50 + 20 * (i - head[nref - 2]), &tmp);
				}
				getBorder(&tmp, nref == 2? 255: 455, 40, nref == 2? 260: 460, 210);
				if(num > 9) drawScroll(offset[nref - 2], num - 1, scx->LN_COLOR, scx->SD_COLOR, &tmp);
				if(num) drawCRect(scx->SD_COLOR, 5, &ttb[offset[nref - 2] - head[nref - 2]], 0);
				u32 free_space, total_space;
				pspIoSpace(dev_item[dev[nref - 2]], &free_space, &total_space);
				char tmps[32];
				char sizes[64];
				getSpaceSize(sizes, free_space);
				getSpaceSize(tmps, total_space);
				strcat(sizes, "/");
				strcat(sizes, tmps);
				drawBText(sizes, scx->TX_COLOR, scx->BG_COLOR, scx->BT_COLOR, (nref == 2? 261 : 461) - 8 * strlen(sizes), 230, &tmp);
				nref ++;
			}
			refresh = 0;
		}
		if(cover)
		{
			drawCRect(scx->BK_COLOR, 20, &tb[neg], 0);
			cover = 0;
		}
		for(i = 0; i < (end[1 - neg] - head[1 - neg]); i ++)
			item[i] = ent[i].d_name;
		tpos = offset[1 - neg] - head[1 - neg];
		key = drawMenu((const char **)item, end[1 - neg] - head[1 - neg], &tpos, neg? 92: 292, 0, 50, 20, KEYSETF, 2);
		offset[1 - neg] = head[1 - neg] + tpos;
		switch(key)
		{
			case 0:
				i = strlen(path[1 - neg]) - 2;
				while(i > 0)
				{
					if(path[1 - neg][i] == '/')
					{
						path[1 - neg][i + 1] = 0;
						reload = 1;
						offset[1 - neg] = 0;
						head[1 - neg] = 0;
						memset(item_stat, 0, 150 * sizeof(u8));
						refresh = 3 - neg;
						break;
					}
					i --;
				}
				if(i == 0)
					done = 1;
				break;
			case 1:
				newhead = min(head[1 - neg] + 1, max(num - 9, 0));
				if(newhead > head[1 - neg])
				{
					head[1 - neg] = newhead;
					offset[1 - neg] ++;
					refresh = 3 - neg;
					update_entry = 1;
				}
				break;
			case 2:
				newhead = max(head[1 - neg] - 1, 0);
				if(newhead < head[1 - neg])
				{
					head[1 - neg] = newhead;
					offset[1 - neg] --;
					refresh = 3 - neg;
					update_entry = 1;
				}
				break;
			case PSP_CTRL_TRIANGLE:
				item_stat[offset[1 - neg]] = 1 - item_stat[offset[1 - neg]];
				refresh = 3 - neg;
				break;
				case PSP_CTRL_SQUARE:
				for(i = 0; i < num; i++)
					item_stat[i] = 1 - item_stat[i];
				refresh = 3 - neg;
				break;
			case PSP_CTRL_LTRIGGER:
				refresh = 1;
				break;
			case PSP_CTRL_RTRIGGER:
				dev[1 - neg] = dev[1 - neg] > 3? 0: (dev[1 - neg] + 1);
				strcpy(path[1 - neg], dev_item[dev[1 - neg]]);
				reload = 1;
				offset[1 - neg] = 0;
				head[1 - neg] = 0;
				memset(item_stat, 0, 150 * sizeof(u8));
				refresh = 3 - neg;
				break;
			case PSP_CTRL_LEFT:
			case PSP_CTRL_RIGHT:
				neg = 1 - neg;
				reload = 1;
				memset(item_stat, 0, 150 * sizeof(u8));
				refresh = 3 - neg;
				cover = 1;
				break;
			case PSP_CTRL_CIRCLE:
			case PSP_CTRL_CROSS:
				if(ent[offset[1 - neg] - head[1 - neg]].d_stat.st_attr & FIO_SO_IFDIR)
				{
					strcat(path[1 - neg], ent[offset[1 - neg] - head[1 - neg]].d_name);
					reload = 1;
					offset[1 - neg] = 0;
					head[1 - neg] = 0;
					memset(item_stat, 0, 150 * sizeof(u8));
					refresh = 3 - neg;
					continue;
				}
				else
				{
					if(filehandler(path[1 - neg], ent[offset[1 - neg] - head[1 - neg]].d_name))
					{
						return 1;
					}
				}
				break;
		}
	}
	getBorder(&tmp, tb[0].x0, tb[0].y0, tb[1].x1, tb[1].y1);
	clearRect(0, &tmp);
	stopUSBHostFSDriver();
	return 0;
}

int module_start (SceSize argc, void* argp) 
{
	return 0;
}

