#include <pspsdk.h>
#include <pspkernel.h>
#include <pspinit.h>
#include <pspcrypt.h>
#include <psploadexec_kernel.h>
#include <psputilsforkernel.h>
#include <psppower.h>
#include <pspreg.h>
#include <pspmediaman.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "sysmodpatches.h"
#include "main.h"
#include "reboot.h"
#include <umd9660_driver.h>
#include <isofs_driver.h>
//#include "virtualpbpmgr.h"
#include "systemctrl_se.h"
#include "malloc.h"

#define EBOOT_BIN "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"
#define BOOT_BIN  "disc0:/PSP_GAME/SYSDIR/BOOT.BIN"

int (* UtilsForKernel_7dd07271)(u8 *dest, u32 destSize, const u8 *src, void *unk, void *unk2);
int (* scePowerSetCpuClockFrequency_k)(int cpufreq);
int (* scePowerSetBusClockFrequency_k)(int busfreq);
int (* scePowerSetClockFrequency_k)(int cpufreq, int ramfreq, int busfreq);
int (* MemlmdDecrypt)(u8 *buf, int size, int *ret, int u);

SEConfig config;
u32 vshmain_args[0x0400/4];

//0x00001608
void SetConfig(SEConfig *newconfig)
{
	memcpy(&config, newconfig, sizeof(SEConfig));
}

//0x00001660
int LoadStartModule(char *module)
{
	SceUID mod = sceKernelLoadModule(module, 0, NULL);

	if (mod < 0)
		return mod;

	return sceKernelStartModule(mod, strlen(module)+1, module, NULL, NULL);
}

//0x000016D0
int ReadLine(SceUID fd, char *str)
{
	char ch = 0;
	int n = 0;

	while (1)
	{
		if (sceIoRead(fd, &ch, 1) != 1)
			return n;

		if (ch < 0x20)
		{
			if (n != 0)
				return n;
		}
		else
		{
			*str++ = ch;
			n++;
		}
	}

}

//0x00001758
u32 FindPowerFunction(u32 nid)
{
	return FindProc("scePower_Service", "scePower", nid);
}

//0x00001770
u32 FindPowerDriverFunction(u32 nid)
{
	return FindProc("scePower_Service", "scePower_driver", nid);
}

//0x00001788
int sceKernelStartModulePatched(SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option)
{
	SceModule *mod = sceKernelFindModuleByUID(modid);
	if (mod != NULL)
	{
		Kprintf(mod->modname);
		if (strcmp(mod->modname, "vsh_module") == 0)
		{
			if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_VSH)
			{
				if (argsize == 0)
				{
					if (config.skiplogo)
					{
						memset(vshmain_args, 0, 0x0400);
						vshmain_args[0/4] = 0x0400;
						vshmain_args[4/4] = 0x20;
						vshmain_args[0x40/4] = 1;
						vshmain_args[0x280/4] = 1;
						vshmain_args[0x284/4] = 0x40003;

						argsize = 0x0400;
						argp = vshmain_args;
					}
				}
			}
		}

		else if (strcmp(mod->modname, "sceMediaSync") == 0)
		{
			char plugin[64];
			int keyconfig, i;
			SceUID fd = -1;

			keyconfig = sceKernelInitKeyConfig();
			
			if(keyconfig == PSP_INIT_KEYCONFIG_POPS)
			{
				fd = sceIoOpen("ms0:/seplugins/pops.txt", PSP_O_RDONLY, 0777);
			}
			else
			{
				int ret = -1;
				while(ret < 0)
				{
					ret = LoadStartModule("flash0:/_real_path_kd/recovery.prx");
					sceKernelDelayThread(200000);
				}
				if (keyconfig == PSP_INIT_KEYCONFIG_VSH)
				{
					fd = sceIoOpen("ms0:/seplugins/vsh.txt", PSP_O_RDONLY, 0777);
				}
				else if(keyconfig == PSP_INIT_KEYCONFIG_GAME)
				{
					fd = sceIoOpen("ms0:/seplugins/game.txt", PSP_O_RDONLY, 0777);
				}
				
			}
			
			if(fd >= 0)
			{
				for(i = 0; i < 10; i++)
				{
					memset(plugin, 0, sizeof(plugin));
					if(ReadLine(fd, plugin) > 0)
					{
						if(plugin[0] != '#')
						{
							LoadStartModule(plugin);
						}
					}
					else
					{
						break;
					}
				}
				sceIoClose(fd);
			}
		}		
	}

	return sceKernelStartModule(modid, argsize, argp, status, option);
}

void *block;
int drivestat = SCE_UMD_READY | SCE_UMD_MEDIA_IN;
SceUID umdcallback;

//0x00001C18
void UmdCallback()
{
	if (umdcallback >= 0)
	{
		sceKernelNotifyCallback(umdcallback, drivestat);
	}
}

//0x00001C3C
int sceUmdActivatePatched(const int mode, const char *aliasname)
{
	int k1 = pspSdkSetK1(0);
	//sceIoAssign(aliasname, block, block+6, IOASSIGN_RDONLY, NULL, 0);
	sceIoAssign(aliasname, "umd0:", "isofs0:", IOASSIGN_RDONLY, NULL, 0);
	pspSdkSetK1(k1);

	drivestat = SCE_UMD_READABLE | SCE_UMD_MEDIA_IN;
	UmdCallback();
	return 0;
}

//0x00001CB0
int sceUmdDeactivatePatched(const int mode, const char *aliasname)
{
	sceIoUnassign(aliasname);
	drivestat = SCE_UMD_MEDIA_IN | SCE_UMD_READY;

	UmdCallback();
	return 0;
}

//0x00001CE0
int sceUmdGetDiscInfoPatched(SceUmdDiscInfo *disc_info)
{
	disc_info->uiSize = 8;
	disc_info->uiMediaType = SCE_UMD_FMT_GAME;

	return 0;
}

//0x00001CF8
int sceUmdRegisterUMDCallBackPatched(SceUID cbid)
{
	umdcallback = cbid;
	UmdCallback();

	return 0;
}

//0x00001D1C
int sceUmdUnRegisterUMDCallBackPatched(SceUID cbid)
{
	umdcallback = -1;
	return 0;
}

//0x00001D30
int sceUmdGetDriveStatPatched()
{
	return drivestat;
}

//0x00001D3C
u32 FindUmdUserFunction(u32 nid)
{
	return FindProc("sceUmd_driver", "sceUmdUser", nid);
}

//0x00001DA0 - patch target updated - checked
void PatchInit(u32 text_addr)
{
	// Patch StartModule
	MAKE_JUMP(text_addr+0x16D4, sceKernelStartModulePatched);
}

//0x00001DCC - patch target updated - checked
void PatchNandDriver(char *buf)
{
	int intr = sceKernelCpuSuspendIntr();
	u32 text_addr = (u32)buf+0xA0;

        //- Security patch: Fixed a bug that has been in all 3.XX OE, that caused random data to be written to a location
        //  in lcdc.prx or (in worst case) emc_sm.prx ram space.
        //changed from 0x0e4c below
	_sh(0xac60, text_addr+0x0E4E);

	sceKernelCpuResumeIntr(intr);
	ClearCaches();

	if (mallocinit() < 0)
	{
		while (1) _sw(0, 0);
	}
}

//0x00001E24 - patch target updated - checked/corrected
void PatchUmdMan(char *buf)
{
	int intr = sceKernelCpuSuspendIntr();
	u32 text_addr = (u32)buf+0xA0;

	if (sceKernelBootFrom() == PSP_BOOT_MS)
	{
		// Replace call to sceKernelBootFrom with return PSP_BOOT_DISC
		_sw(0x24020020, text_addr+0x13F4);
	}

	sceKernelCpuResumeIntr(intr);
	ClearCaches();
}

//0x00001AF0 - offsets checked
int LoadRebootex(u8 *dest, u32 destSize, const u8 *src, void *unk, void *unk2)
{
	u8 *output = (u8 *)0x88fc0000;
	char *theiso;
	int i;

	theiso = GetUmdFile();
	if (theiso)
	{
		strcpy((void *)0x88fb0000, theiso);
	}

	memcpy((void *)0x88fb0050, &config, sizeof(SEConfig));

	for (i = 0; i < (sizeof(rebootex)-0x10); i++)
	{
		output[i] = rebootex[i+0x10];
	}

	return UtilsForKernel_7dd07271(dest, destSize, src, unk, unk2);
}

//0x00001E78 - patch targets unchanged - checked
void PatchLoadExec(u32 text_addr)
{
	_sw(0x3C0188fc, text_addr+0x16E4);
	MAKE_CALL(text_addr+0x16A8, LoadRebootex);
	UtilsForKernel_7dd07271 = (void *)(text_addr+0x1BCC);

	// Allow LoadExecVSH in whatever user level
	_sw(0x1000000b, text_addr+0x0F38);
	_sw(0, text_addr+0x0F78);

	// Allow ExitVSHVSH in whatever user level
	_sw(0x10000008, text_addr+0x0720);
	_sw(0, text_addr+0x0754);
}

//0x00001ED8 - patch targets unchanged - checked
void PatchInitLoadExecAndMediaSync(char *buf)
{
	int intr = sceKernelCpuSuspendIntr();

	char *filename = sceKernelInitFileName();
	u32 text_addr = (u32)buf+0xA0;

	if (filename)
	{
		if (strstr(filename, ".PBP"))
		{
			// Patch mediasync (avoid error 0x80120005 sfo error)
			// Make return 0
			_sw(0x00008021, text_addr+0x5B8);
			ClearCaches();

			SetUmdFile("");
		}

		else if (strstr(filename, "disc") == filename)
		{
			if (!config.umdactivatedplaincheck)
				UndoSuperNoPlainModuleCheckPatch();

			char *theiso = GetUmdFile();

			if (theiso)
			{
				if (strncmp(theiso, "ms0:/", 5) == 0)
				{
					if (!config.usenoumd && !config.useisofsonumdinserted)
					{
						DoAnyUmd();
					}
					else
					{
						sceKernelCpuResumeIntr(intr);
						sceIoDelDrv("umd");
						sceIoAddDrv(getumd9660_driver());
						intr = sceKernelCpuSuspendIntr();
						//DoAnyUmd();
					}

					if (config.usenoumd)
					{
						// Patch the error of no disc
						_sw(0x34020000, text_addr+0xEC);
					}
					else
					{
						// Patch the error of diferent sfo?
						_sw(0x00008021, text_addr+0xF8);
					}
				}
				else
				{
					SetUmdFile("");
				}
			}
		}
		else
		{
			SetUmdFile("");
		}
	}
	else
	{
		SetUmdFile("");
	}

	u32 *mod = (u32 *)sceKernelFindModuleByName("sceInit");
	if (mod)
	{
		PatchInit(*(mod+27));
	}

	mod = (u32 *)sceKernelFindModuleByName("sceLoadExec");
	if (mod)
	{
		PatchLoadExec(*(mod+27));
	}	

	sceKernelCpuResumeIntr(intr);
	ClearCaches();

	int (* sceClockgen_driver_5F8328FD) (void) = (void *)FindProc("sceClockgen_Driver", "sceClockgen_driver", 0x5F8328FD);

	sceClockgen_driver_5F8328FD();
}

//0x000020B8 - patch targets updated - checked
void PatchIsofsDriver(char *buf)
{
	// Patch StopModule to avoid crash at exit...
	//u32 *mod = (u32 *)sceKernelFindModuleByName("sceIsofs_driver");
	//u32 text_addr = *(mod+27);
	u32 text_addr = (u32)(buf+0xA0);

	char *iso = GetUmdFile();

	if (iso)
	{
		if (strstr(iso, "ms0:/") == iso)
		{
			if (config.usenoumd || config.useisofsonumdinserted)
			{
				/* make module exit inmediately */
				int intr = sceKernelCpuSuspendIntr();
				_sw(0x03E00008, text_addr+0x4320);
				_sw(0x24020001, text_addr+0x4324);
				sceKernelCpuResumeIntr(intr);

				ClearCaches();
			}
		}
	}

	/*_sw(0x03e00008, text_addr+0x4a2c);
	_sw(0x34020000, text_addr+0x4a30);*/
}

//0x0000214C - patches added/checked
void PatchPower(char *buf)
{
	u32 text_addr = (u32)(buf+0xA0);

	_sw(0, text_addr+0xB24);

	//new to 3.40:
	_sw(0, text_addr+0x1D74);
	_sw(0, text_addr+0x24EC);
	_sw(0, text_addr+0x24F4);
	_sw(0, text_addr+0x24FC);
	_sw(0, text_addr+0x2504);
	_sw(0, text_addr+0x25A8);
	_sw(0, text_addr+0x25BC);
	_sw(0, text_addr+0x25C4);
	_sw(0, text_addr+0x25CC);

	ClearCaches();
}

//0x0000217C - patches unchanged - checked
void PatchWlan(char *buf)
{
	u32 text_addr = (u32)(buf+0xA0);

	_sw(0, text_addr+0x2520);
	ClearCaches();
}

//0x00002188
void DoNoUmdPatches()
{
	REDIRECT_FUNCTION(FindUmdUserFunction(0xC6183D47), sceUmdActivatePatched);
	REDIRECT_FUNCTION(FindUmdUserFunction(0xE83742BA), sceUmdDeactivatePatched);
	REDIRECT_FUNCTION(FindUmdUserFunction(0x340B7686), sceUmdGetDiscInfoPatched);
	REDIRECT_FUNCTION(FindUmdUserFunction(0xAEE7404D), sceUmdRegisterUMDCallBackPatched);
	REDIRECT_FUNCTION(FindUmdUserFunction(0xBD2BDE07), sceUmdUnRegisterUMDCallBackPatched);
	MAKE_DUMMY_FUNCTION1(FindUmdUserFunction(0x46EBB729)); // sceUmdCheckMedium
	MAKE_DUMMY_FUNCTION0(FindUmdUserFunction(0x8EF08FCE)); // sceUmdWaitDriveStat
	MAKE_DUMMY_FUNCTION0(FindUmdUserFunction(0x56202973)); // sceUmdWaitDriveStatWithTimer
	MAKE_DUMMY_FUNCTION0(FindUmdUserFunction(0x4A9E5E29)); // sceUmdWaitDriveStatCB
	REDIRECT_FUNCTION(FindUmdUserFunction(0x6B4A146C), sceUmdGetDriveStatPatched);
	MAKE_DUMMY_FUNCTION0(FindUmdUserFunction(0x20628E6F)); // sceUmdGetErrorStat

	ClearCaches();
}

//0x00001D54
int sceChkregGetPsCodePatched(u8 *pscode)
{
	pscode[0] = 0x01;
	pscode[1] = 0x00;
	pscode[2] = config.fakeregion + 2;

	if (pscode[2] == 2)
		pscode[2] = 3;

	pscode[3] = 0x00;
	pscode[4] = 0x01;
	pscode[5] = 0x00;
	pscode[6] = 0x01;
	pscode[7] = 0x00;

	return 0;
}

//0x000023A4
void PatchChkreg()
{
	u32 pscode = FindProc("sceChkreg", "sceChkreg_driver", 0x59F8491D);
	int (* sceChkregGetPsCode)(u8 *);
	u8 code[8];

	int intr = sceKernelCpuSuspendIntr();

	if (pscode)
	{
		if (config.fakeregion)
		{
			REDIRECT_FUNCTION(pscode, sceChkregGetPsCodePatched);
		}
		else
		{
			sceChkregGetPsCode = (void *)pscode;

			memset(code, 0, 8);
			sceChkregGetPsCode(code);

			if (code[2] == 0x06 || code[2] == 0x0A || code[2] == 0x0B
				|| code[2] == 0x0D)
			{
				REDIRECT_FUNCTION(pscode, sceChkregGetPsCodePatched);
			}
		}
	}

	/*u32 checkregion = FindProc("sceChkreg", "sceChkreg_driver", 0x54495B19);

	if (checkregion)
	{
		if (config.freeumdregion)
		{
			MAKE_DUMMY_FUNCTION1(checkregion);
		}
	}*/

	sceKernelCpuResumeIntr(intr);
	ClearCaches();
}

//0x00002498
void SetSpeed(int cpu, int bus)
{
	scePowerSetClockFrequency_k = (void *)FindPowerDriverFunction(0x545A7F3C);
	scePowerSetClockFrequency_k(cpu, cpu, bus);

	int intr = sceKernelCpuSuspendIntr();

	MAKE_DUMMY_FUNCTION0((u32)scePowerSetClockFrequency_k);
	MAKE_DUMMY_FUNCTION0((u32)FindPowerDriverFunction(0x737486F2));
	MAKE_DUMMY_FUNCTION0((u32)FindPowerDriverFunction(0xB8D7B3FB));
	MAKE_DUMMY_FUNCTION0((u32)FindPowerDriverFunction(0x843FBF43));

	sceKernelCpuResumeIntr(intr);
	ClearCaches();
}

//0x00002580
void OnImposeLoad()
{
	u32 *mod = (u32 *)sceKernelFindModuleByName("sceChkreg");

	if (mod)
	{
		sctrlSEGetConfig(&config);
		PatchChkreg();
	}

	if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_GAME)
	{
		char *theiso = GetUmdFile();

		if (theiso)
		{
			if (strncmp(theiso, "ms0:/", 5) == 0)
			{
				if (config.usenoumd || config.useisofsonumdinserted)
				{
					sceIoDelDrv("isofs");
					sceIoAddDrv(getisofs_driver());

					//PatchIsofsDriver();

					if (config.usenoumd)
					{
						DoNoUmdPatches();
					}

					sceIoAssign("disc0:", "umd0:", "isofs0:", IOASSIGN_RDONLY, NULL, 0);
				}
			}
		}
	}

	if (sceKernelInitApitype() == PSP_INIT_APITYPE_DISC)
	{
		if (config.umdisocpuspeed == 333 || config.umdisocpuspeed == 300 ||
			config.umdisocpuspeed == 266 || config.umdisocpuspeed == 222)
			SetSpeed(config.umdisocpuspeed, config.umdisobusspeed);
	}

        //missing from 3.40 asm
	//PatchMemlmd();
}
