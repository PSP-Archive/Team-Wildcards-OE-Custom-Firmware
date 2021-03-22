// 3.40 OE SystemControl150 (flash0:/kd/rtc.prx)
//
// Reversed Pseudo C 
// Original code by Dark_AleX (c)(2007)
//
// Working copy
//

#include <pspsdk.h>
#include <pspkernel.h>
#include <psploadexec.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <pspctrl.h>

#include "rebootbin.h"
#include "rebootexbin.h"

PSP_MODULE_INFO("SystemControl150", 0x3007, 1, 0);

#define FALSE (0)
#define TRUE (1)

#define RECOVERY_PROGRAM "flash0:/kd/recovery.prx"
#define AUTOBOOT_PROGRAM "ms0:/PSP/GAME/BOOT/EBOOT.PBP"

#define REBOOT_SIZE (0xB0C8) // 45,256 Bytes
#define REBOOTEX_SIZE (0xA38) // 2616 Bytes

#define MIPS_NOP 0x00000000
#define MIPS_JAL 0x0C000000

#define MIPS_LUI(RD,IMM)  (0x3c000000|(RD<<16)|((u32)(IMM)&0xffff))
#define PATCH(ADDR, FUNC) _sw(MIPS_JAL | (((u32)(FUNC) & 0x3FFFFFFF) >> 2), ADDR)

extern int pspSdkInstallNoDeviceCheckPatch(void);
extern int pspSdkInstallNoPlainModuleCheckPatch(void);

// I havent updated my SDK, these arent defined
// if you an updated one which defines these then remove
void sceKernelDcacheWritebackAll(void);
void* sceKernelMemset(void *dst ,int value ,size_t size); 
void sceKernelExitVSHVSH(u32 *unk);
int sceKernelDeflateDecompress(u8 *dst, u32 dstSize, const void *src, u32 end);

void clear_cache(void);
int LoadExecAction_fake(int apiType, SceSize bufSize, const char *file, struct SceKernelLoadExecParam *param, int unk);
void *sceKernelMemset_fake(void *inaddr, int inval, int insize);
int sceKernelGzipDecompress_fake(u8 *dst, u32 dstSize, const void *src, u32 crc32);

int isReboot;
u32 (* LoadExecAction_real)(int apiType, SceSize bufSize, const char *file, struct SceKernelLoadExecParam *param, int unk);



void clear_cache(void)
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheInvalidateAll();
}


// Our Fake patch functions:

int sceKernelGzipDecompress_fake(u8 *dst, u32 dstSize, const void *src, u32 crc32)
{
	int i=0;
	u8 *rebootex = (u8 *)0x88FC0000;

	// Copy rebootex.bin to 0x88FC0000
	for (i=0; i!=REBOOTEX_SIZE; i++)
		rebootex[i] = rebootex_bin[i];

	if (isReboot)
	{
		// decompress 3.40 reboot.bin to 0x88600000
		return(sceKernelDeflateDecompress((void *)0x88600000, dstSize, reboot_bin, crc32));
	}
	else
	{
		return(sceKernelGzipDecompress(dst, dstSize, src, crc32));
	}
}

int LoadExecAction_fake(int apiType, SceSize bufSize, const char *file, struct SceKernelLoadExecParam *param, int unk)
{
	if (apiType & 0x200) // sceKernelExitVSHVSH or sceKernelExitGame
	{
		isReboot = TRUE;
	}

	return(LoadExecAction_real(apiType, bufSize, file, param, unk));
}

void *sceKernelMemset_fake(void *inaddr, int inval, int insize)
{
	void *addr = inaddr;
	int val = inval;
	int size = insize;

	if (isReboot)
	{
		sceKernelSetDdrMemoryProtection((void *)0x88400000, 0x400000, 0xC); // unlock extra 4MB ?

		addr = (void *)0x88600000; // 3.40 reboot.bin start address
		size = 0x200000;
	}

	return(sceKernelMemset(addr, val, size));
}

int module_start(SceSize args, void *argp) __attribute__((alias("_start")));

int _start(SceSize args, void *argp)
{
	pspSdkInstallNoDeviceCheckPatch();
	pspSdkInstallNoPlainModuleCheckPatch();

	//SceModule *modinfo;
	u32 *modinfo;
	u32 text_addr;

	// patch modulemanager
	modinfo = (u32 *)sceKernelFindModuleByName("sceModuleManager");
	if (modinfo)
	{
		text_addr = *(modinfo+27);
		// Patch within ReadFile func

		// Patch filesize check
		_sw(0x1000002A, text_addr+0x3F28);
	}

	// patch loadexec
	modinfo = (u32 *)sceKernelFindModuleByName("sceLoadExec");	
	if (modinfo)
	{
		text_addr = *(modinfo+27);
		LoadExecAction_real = (void *)(text_addr+0x2138); // location of real func

		// Patch within LoadExecBody func

		// Replace LoadExecAction func with our own
		PATCH(text_addr+0x2090, LoadExecAction_fake);

		// Patch within LoadExecAction func

		// Replace sceKernelMemset & sceKernelGzipDecompress func with our own
		PATCH(text_addr+0x232C, sceKernelMemset_fake);
		PATCH(text_addr+0x2344, sceKernelGzipDecompress_fake);

		// Replace call to 1.50 reboot.bin to our own rebootex.bin
		// Original 1.50 reboot.bin at 0x88C00000, our rebootex.bin at 0x88FC0000
		_sw(MIPS_LUI(1, 0x88FC), text_addr+0x2384);
	}

	clear_cache();

	if (sceKernelFindModuleByName("lcpatcher"))
	{
		struct SceKernelLoadExecParam param;
		SceCtrlData pad;

		sceCtrlPeekBufferPositive(&pad, 1);

		// To run recovery
		if (pad.Buttons & PSP_CTRL_RTRIGGER)
		{
			param.size = sizeof(param);
			param.args = 0;
			param.argp = NULL;
			param.key = "updater";

			sceKernelLoadExec(RECOVERY_PROGRAM, &param);

			return 0;
		}

		// The following 3 lines will be patched out by recovery
		// when autoboot is enabled to allow the loadexec to run

		// Placeholder for patch
		asm("nop \n");
		asm("nop \n");

		// Boot into OE
		sceKernelExitVSHVSH(NULL); // Should call our fake rebootex.bin

		// The following only runs when autoboot has been enabled
		param.size = sizeof(param);
		param.args = 29; 
		param.argp = AUTOBOOT_PROGRAM;
		param.key = "game";

		sceKernelLoadExec(AUTOBOOT_PROGRAM, &param);
	}

	return 0;
}

int module_stop(SceSize args, void *argp) __attribute__((alias("_stop")));

int _stop(SceSize args, void *argp)
{
	return 0;
}



