#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <psplflash_fatfmt.h>
#include <pspctrl.h>
#include <psppower.h>
#include <psputility.h>
#include <pspsyscon.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>

#include <systemctrl_se.h>
#include "dxar.h"

PSP_MODULE_INFO("DXARFlasher", 0x1000, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

#define printf pspDebugScreenPrintf

void ErrorExit(int milisecs, char *fmt, ...)
{
	va_list list;
	char msg[256];	

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	printf(msg);

	sceKernelDelayThread(milisecs*1000);
	sceKernelExitGame();
}

u32 FindProc(const char* szMod, const char* szLib, u32 nid)
{
	struct SceLibraryEntryTable *entry;
	SceModule *pMod;
	void *entTab;
	int entLen;

	pMod = sceKernelFindModuleByName(szMod);

	if (!pMod)
	{
		//***printf("Cannot find module %s\n", szMod);
		return 0;
	}
	
	int i = 0;

	entTab = pMod->ent_top;
	entLen = pMod->ent_size;
	//***printf("entTab %p - entLen %d\n", entTab, entLen);
	while(i < entLen)
    {
		int count;
		int total;
		unsigned int *vars;

		entry = (struct SceLibraryEntryTable *) (entTab + i);

        if(entry->libname && !strcmp(entry->libname, szLib))
		{
			total = entry->stubcount + entry->vstubcount;
			vars = entry->entrytable;

			if(entry->stubcount > 0)
			{
				for(count = 0; count < entry->stubcount; count++)
				{
					if (vars[count] == nid)
						return vars[count+total];					
				}
			}
		}

		i += (entry->len * 4);
	}

	//***printf("Funtion not found.\n");
	return 0;
}

u8 *dxarbuf;
u8 *filebuf;

int WriteFile(char *file, void *buf, int size)
{
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	if (fd < 0)
	{
		return -1;
	}

	int written = sceIoWrite(fd, buf, size);
	
	if (sceIoClose(fd) < 0)
		return -1;

	return written;
}

void ReadSection(char *section)
{
	int size;	

	size = dxarOpenSection(section);
	
	if (size <= 0)
	{
		ErrorExit(6000, "Error (dxar open).\n");
	}

	if (size > 17000000)
	{
		ErrorExit(6000, "Section too big.\n");
	}	

	if (dxarReadSection(dxarbuf) < 0)
	{
		ErrorExit(6000, "Error (dxar readsect).\n");
	}	
}

void SetPercentage(int x, int y, int value, int max, int dv)
{
	if (dv)
	{
		value /= dv;
		max /= dv;
	}

	pspDebugScreenSetXY(x, y);
	printf("%3d%%", ((100 * value) / max));
}

void WriteFiles()
{
	DXAR_FileEntry entry;
	int res;
	int x, y;
	int pos, max;

	x = pspDebugScreenGetX();
	y = pspDebugScreenGetY();
	
	while (dxarGetNextFile(&entry, filebuf, &pos, &max) == 0)
	{
		if ((res = WriteFile(entry.filepath, filebuf, entry.filesize)) < 0)
		{
			printf("PANIC: error 0x%08X writing file %s\n", res, entry.filepath);
		}

		SetPercentage(x, y, pos, max, 0);

		sceIoSync("flash0:", 0x10);
		scePowerTick(0);
	}

	SetPercentage(x, y, 1, 1, 0);
	printf("\n");
}

void Agreement()
{
	SceCtrlData pad;

	printf("You are about to flash your PSP to custom firm 3.10 OE-A.\n");
	printf("Press X to start. By doing it, you accept the risk and ALL the responsability of what happens.\n");
	printf("If you don't agree press R button.\n\n");

	while (1)
	{
		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS)
		{
			return;			
		}

		else if (pad.Buttons & PSP_CTRL_RTRIGGER)
		{
			sceKernelExitGame();
		}

		sceKernelDelayThread(10000);
	}
}

int ByPass()
{
	SceCtrlData pad;

	sceCtrlReadBufferPositive(&pad, 1);

	if (pad.Buttons & PSP_CTRL_LTRIGGER)
	{
		if (pad.Buttons & PSP_CTRL_TRIANGLE)
		{
			return 1;
		}
	}	

	return 0;
}

int main()
{
	DXAR_FileEntry entry;	
	char *argv[2];
	
	pspDebugScreenInit();
	pspDebugScreenSetTextColor(0x005435D0);

	if (!ByPass())
	{
		if (scePowerGetBatteryLifePercent() < 75)
		{
			ErrorExit(6000, "Battery has to be at least at 75%%.\n");
		}
	}

	Agreement();
	//LammerCheck();
		
	dxarbuf = (u8 *)malloc(17000000);

	if (!dxarbuf)
	{
		ErrorExit(6000, "Memory allocation error.\n");
	}

	filebuf = (u8 *)malloc(2000000);

	if (!filebuf)
	{
		ErrorExit(6000, "Memory allocation error.\n");
	}

	printf("Opening and validating dxar file...");

	if (dxarOpenAndValidate("DATA.DXAR", dxarbuf, 17000000) < 0)
	{
		ErrorExit(6000, "Not existing or corrupted dxar file.\n");
	}

	if (pspSdkLoadStartModule("flash0:/kd/lflash_fatfmt.prx", PSP_MEMORY_PARTITION_KERNEL) < 0)
		ErrorExit(6000, "Cannot load lflash_fatfmt.\n");

	printf(" done.\n");

	printf("Logical formating flash0...");

	if (sceIoUnassign("flash0:") < 0)
		ErrorExit(6000, "Cannot unassign flash0. Exiting.\n");

	if (sceIoUnassign("flash1:") < 0)
		ErrorExit(6000, "Cannot unassign flash1. Exiting.\n");

	argv[0] = "fatfmt";
	argv[1] = "lflash0:0,0";
	int x;

	if ((x = sceLflashFatfmtStartFatfmt(2, argv)) < 0)
	{
		ErrorExit(7000, "PANIC: Error formating flash0 %08X.\n", x);
	}

	if (sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, NULL, 0) < 0)
	{
		printf("PANIC: error re-assigning flash0.\n");
	}

	if (sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, NULL, 0) < 0)
	{
		printf("PANIC: error re-assigning flash1.\n");
	}

	printf(" done.\n");

	printf("Creating directories...");

	ReadSection("DIR");

	while (dxarGetNextFile(&entry, filebuf, NULL, NULL) == 0)
	{
		if (sceIoMkdir(entry.filepath, 0777) < 0)
		{
			printf("PANIC: error creating directory %s\n", entry.filepath);
		}
	}

	if (dxarCloseSection() < 0)
	{
		ErrorExit(6000, "Error (dxar).\n");
	}

	printf(" done.\n");

	printf("Writing 1.50 subset... ");

	ReadSection("1.50");
	WriteFiles();

	if (dxarCloseSection() < 0)
	{
		ErrorExit(6000, "Error (dxar close).\n");
	}

	printf("Writing 3.10... ");
	ReadSection("3.10");
	WriteFiles();

	if (dxarCloseSection() < 0)
	{
		ErrorExit(6000, "Error (dxar).\n");
	}

	if (dxarClose() < 0)
	{
		ErrorExit(6000, "Error (dxar).\n");
	}

	printf("Disabling plugins... ");

	u8 conf[15];

	memset(conf, 0, 15);
	
	WriteFile("ms0:/seplugins/conf.bin", conf, 15);

	printf("done.\n");


	/*printf("Writing default configuration...\n");

	SEConfig config;

	memset(&config, 0, sizeof(config));
	config.umdactivatedplaincheck = 1;
	
	SE_SetConfig(&config);*/

	printf("Done. Press X to shutdown the PSP. Restart manually.");
	
	while (1)
	{
		SceCtrlData pad;

		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS)
		{
			break;		
		}
		
		sceKernelDelayThread(10000);
	}

	sceSysconPowerStandby();

	return 0;
}
