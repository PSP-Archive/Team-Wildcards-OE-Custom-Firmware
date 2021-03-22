#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman_kernel.h>
#include "main.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "sysmodpatches.h"
#include "umd9660_driver.h"
#include "isofs_driver.h"

//0x00005F14 - target updated for 3.40 - checked
int sctrlKernelSetUserLevel(int level)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelGetUserLevel();

	u32 *mod = (u32 *)sceKernelFindModuleByName("sceThreadManager");
	u32 text_addr;
	u32 *thstruct;

	if (!mod)
	{
		pspSdkSetK1(k1);
		return -1;
	}

	text_addr = *(mod+27);
	thstruct = (u32 *)_lw(text_addr+0x174C4);

	thstruct[0x14/4] = (level ^ 8) << 28;

	pspSdkSetK1(k1);
	return res;
}

//0x00005FA8
int	sctrlHENIsSE()
{
	return 1;
}

//0x00005FB0
int	sctrlHENIsDevhook()
{
	return 0;
}

//0x00005FB8
int sctrlHENGetVersion()
{
	return 0x00000500;
}

//0x00005FC0
int sctrlSEGetVersion()
{
	return 0x00000600;
}

//0x00005FC8
int sctrlKernelLoadExecVSHDisc(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1;
	int res;

	k1 = pspSdkSetK1(0);
	res = sceKernelLoadExecVSHDisc(file, param);

	pspSdkSetK1(k1);
	return res;
}

//0x00006024
int sctrlKernelLoadExecVSHDiscUpdater(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1;
	int res;

	k1 = pspSdkSetK1(0);
	res = sceKernelLoadExecVSHDiscUpdater(file, param);

	pspSdkSetK1(k1);
	return res;
}

//0x00006080
int sctrlKernelLoadExecVSHMs1(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1;
	int res;

	k1 = pspSdkSetK1(00);
	res = sceKernelLoadExecVSHMs1(file, param);

	pspSdkSetK1(k1);
	return res;
}

//0x000060DC
int sctrlKernelLoadExecVSHMs2(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1;
	int res;

	k1 = pspSdkSetK1(0);
	res = sceKernelLoadExecVSHMs2(file, param);

	pspSdkSetK1(k1);
	return res;
}

//0x0x00006138
int sctrlKernelLoadExecVSHMs3(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1;
	int res;

	k1 = pspSdkSetK1(0);	
	res = sceKernelLoadExecVSHMs3(file, param);
	pspSdkSetK1(k1);

	return res;
}

//0x00006194
int sctrlKernelLoadExecVSHMs4(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1;
	int res;

	k1 = pspSdkSetK1(0);
	res = sceKernelLoadExecVSHMs4(file, param);
	pspSdkSetK1(k1);

	return res;
}

//0x000061F0
int sctrlKernelLoadExecVSHWithApitype(int apitype, const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1;
	int res;
	int (* LoadExecVSH)(int apitype, const char *file, struct SceKernelLoadExecVSHParam *param, int unk2);

	k1 = pspSdkSetK1(0);

	u32 *mod = (u32 *)sceKernelFindModuleByName("sceLoadExec");
	u32 text_addr;

	if (!mod)
		return -1;

	text_addr = *(mod+27);
	LoadExecVSH = (void *)(text_addr+0xEE8);

	res = LoadExecVSH(apitype, file, param, 0x10000);

	pspSdkSetK1(k1);

	return res;
}

//0x00006280
int sctrlKernelExitVSH(struct SceKernelLoadExecVSHParam *param)
{
	int k1;
	int res;

	k1 = pspSdkSetK1(0);
	res = sceKernelExitVSHVSH(param);

	pspSdkSetK1(k1);
	return res;
}

//0x000062CC - target updated for 3.40
PspIoDrv *sctrlHENFindDriver(char *drvname)
{
	int k1 = pspSdkSetK1(0);
	u32 *mod = (u32 *)sceKernelFindModuleByName("sceIOFileManager");

	if (!mod)
	{
		pspSdkSetK1(k1);
		return NULL;
	}

	u32 text_addr = *(mod+27);

	u32 *(* GetDevice)(char *) = (void *)(text_addr+0x2800);
	u32 *u;

	u = GetDevice(drvname);

	if (!u)
	{
		pspSdkSetK1(k1);
		return NULL;
	}

	return (PspIoDrv *)u[1];
}

//0x00006350 - target updated for 3.40 - checked
int sctrlKernelSetInitApitype(int apitype)
{
	int k1 = pspSdkSetK1(0);
	int prev = sceKernelInitApitype();

	u32 *mod = (u32 *)sceKernelFindModuleByName("sceInit");
	u32 text_addr;

	if (!mod)
	{
		pspSdkSetK1(k1);
		return -1;
	}

	text_addr = *(mod+27);
	_sw(apitype, text_addr+0x1E50);

	pspSdkSetK1(k1);
	return prev;
}

//0x000063D0 - target updated for 3.40 - checked
int sctrlKernelSetInitFileName(char *filename)
{
	int k1 = pspSdkSetK1(0);

	u32 *mod = (u32 *)sceKernelFindModuleByName("sceInit");
	u32 text_addr;

	if (!mod)
	{
		pspSdkSetK1(k1);
		return -1;
	}

	text_addr = *(mod+27);
	_sw((u32)filename, text_addr+0x1E74);

	pspSdkSetK1(k1);
	return 0;
}

//0x00006440 - target updated for 3.40 - checked
int sctrlKernelSetInitKeyConfig(int key)
{
	int k1 = pspSdkSetK1(0);
	int prev = sceKernelInitKeyConfig();

	u32 *mod = (u32 *)sceKernelFindModuleByName("sceInit");
	u32 text_addr;

	if (!mod)
	{
		pspSdkSetK1(k1);
		return -1;
	}

	text_addr = *(mod+27);
	_sw(key, text_addr+0x1FC0);

	pspSdkSetK1(k1);
	return prev;
}

//inline @ 0x00006558 - target updated for 3.40 - checked/corrected
static void PatchIsofsDriver2()
{
	u32 *mod = (u32 *)sceKernelFindModuleByName("sceIsofs_driver");

	if (mod)
	{
		u32 text_addr = *(mod+27);

                //3.40: moved to start of module_reboot_before
		_sw(0x03e00008, text_addr+0x4388); //4B14);
		_sw(0x34020000, text_addr+0x438C); //4B18);
		ClearCaches();
	}
}

//0x000064C0
int sctrlSEMountUmdFromFile(char *file, int noumd, int isofs)
{
	int k1 = pspSdkSetK1(0);
	int res;

	SetUmdFile(file);

	if (!noumd && !isofs)
	{
		DoAnyUmd();
	}

	else
	{
		if ((res = sceIoDelDrv("umd")) < 0)
			return res;

		if ((res = sceIoAddDrv(getumd9660_driver())) < 0)
			return res;
	}

	if (noumd)
	{
		DoNoUmdPatches();
	}

	if (isofs)
	{
		sceIoDelDrv("isofs");
		sceIoAddDrv(getisofs_driver());
		PatchIsofsDriver2();

		sceIoAssign("disc0:", "umd0:", "isofs0:", IOASSIGN_RDONLY, NULL, 0);
	}

	pspSdkSetK1(k1);
	return 0;
}

//0x00006600
int sctrlKernelSetDevkitVersion(int version)
{
	int k1 = pspSdkSetK1(0);
	int prev = sceKernelDevkitVersion();

	int high = version >> 16;
	int low = version & 0xFFFF;

	_sh(high, 0x8800E960);
	_sh(low, 0x8800E968);

	ClearCaches();

	pspSdkSetK1(k1);
	return prev;
}
