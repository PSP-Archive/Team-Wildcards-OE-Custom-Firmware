
#include <pspkernel.h>
#include <pspsdk.h>
#include <pspdisplay.h>
#include <pspreg.h>
#include <psppower.h>
#include <psputilsforkernel.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PSP_MODULE_INFO("CXMB", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

#define MIPS_J_ADDRESS(x) (((u32)(x) & 0x3FFFFFFF) >> 2)
#define NOP       (0x00000000)
#define JAL_TO(x) (0x0E000000 | MIPS_J_ADDRESS(x))
#define J_TO(x)   (0x08000000 | MIPS_J_ADDRESS(x))
#define LUI(x,y)  (0x3C000000 | ((x & 0x1F) << 0x10) | (y & 0xFFFF))

static u32 * pspModuleExportHelper(SceModule* mod, const char* lib, u32 nid, u32 * newProcAddr)
{
	u32 * ent_next = (u32 *) mod->ent_top;
	u32 * ent_end  = (u32 *) mod->ent_top + (mod->ent_size >> 2);

	while (ent_next < ent_end)
	{
		SceLibraryEntryTable* ent = (SceLibraryEntryTable*)ent_next;

		if (ent->libname && strcmp(ent->libname, lib) == 0)
 		{
			int count = ent->stubcount + ent->vstubcount;
			u32* nidtable = (u32*)ent->entrytable;
			int i;
			for (i = 0; i < count; i++)
			{
				if (nidtable[i] == nid)
 				{
 					u32 * procAddr =(u32 *)nidtable[count+i];
     				if (newProcAddr)
					{
						nidtable[count+i] = (u32)newProcAddr;
					}
					return procAddr;
				} 
			} 
			return (u32 *)0; 
		} 
		ent_next +=  ent->len;  // len in 32-bit words. 
	} 
	return (u32 *)0;
}

static u32 * pspModuleImportHelper(SceModule* mod, u32 nid, u32 * newCallAddr) 
{ 
	u32 * stub_next =  (u32 *)mod->stub_top;
	u32 * stub_end  =  (u32 *)mod->stub_top + (mod->stub_size >> 2); 

	while (stub_next < stub_end) 
	{ 
		SceLibraryStubTable* stub = (SceLibraryStubTable*) stub_next; 
		u32* table = stub->stubtable; 
		int n = stub->stubcount; 
		int j; 
		for (j = 0; j < n; j++) 
		{ 
			if (stub->nidtable[j] == nid) 
			{
				u32 * callAddr = ((u32 *) &(table[j << 1]));
				if(newCallAddr)
				{
					u32 I = J_TO(newCallAddr);
					int intc = pspSdkDisableInterrupts();
					_sw(I, (u32)callAddr);
					_sw(NOP, (u32)(callAddr + 0x01));
					sceKernelDcacheWritebackAll();
					sceKernelIcacheInvalidateAll();
					pspSdkEnableInterrupts(intc);
				}
		    	return callAddr;
			} 
		} 
		stub_next += stub->len; // len in 32-bit words.
	} 
	return (u32 *)0; 
}

static void * pspHookMethod(const char * mod_name, const char * lib_name, u32 nid, void * method_patch, int flag)
{
	SceModule *pMod;
	SceUID mods[100];
	memset(mods, 0, sizeof(SceUID) * 100);
	int mod_count, i;
	pMod = sceKernelFindModuleByName(mod_name);
	if(!pMod)
		return NULL;
	u32 * fnc = pspModuleExportHelper(pMod, lib_name, nid, (u32 *)method_patch);
	
	if(flag)
	{
		sceKernelGetModuleIdList(mods, 100 * sizeof(SceUID), &mod_count);
		for(i = 0; i < mod_count; i++)
		{
			pMod = sceKernelFindModuleByUID(mods[i]);
			if(pMod)
				pspModuleImportHelper(pMod, nid, (u32 *)method_patch);
		}
	}
	return (void *)fnc;
}

#define U32V(x) (*(u32 *)(x))

static const char * filename[] =
{
	"visualizer_plugin.rco",
	"gameboot.pmf",
	"opening_plugin.rco",
	"01-12.bmp",
	"game_plugin.rco",
	"impose_plugin.rco",
	"photo_plugin.rco",
	"msvideo_main_plugin.rco",
	"system_plugin_bg.rco",
	"system_plugin_fg.rco",
	"system_plugin.rco",
	"sysconf_plugin.rco",
	"topmenu_plugin.rco"
};

static u8 patchfile[13];

static u8 font_patch = 0;

SceUID (* sceIoOpen_ori)(const char * file, int flag, SceMode mode);

int (* sceIoClose_ori)(SceUID fd);

/*static int openlog()
{
    return sceIoOpen("ms0:/sysdump.txt", PSP_O_CREAT | PSP_O_APPEND | PSP_O_RDWR, 0777);
}
static void printlog(int fd, const char* sz)
{
    sceIoWrite(fd, sz, strlen(sz));
}
static void closelog(int fd)
{
    sceIoClose(fd);
}*/

static int cstrcmp(const char * str1, const char * str2)
{
	int j;
	for(j = 0; j < strlen(str2); j++)
	{
		if(str1[j] != str2[j])
			return 0;
	}
	return 1;
}

static int checkName(const char * name, int num)
{
	int i;
	for(i = 0; i < num; i++)
	{
		if(patchfile[i] && cstrcmp(name, filename[i]))
			return 1;
	}
	return 0;
}

static int cxmbPatchMemory(u32 top, int size, int num)
{
	int count = 0;
	u32 addr = top;
	while(addr < (top + size))
	{
		if(U32V(addr) == 0x6F736572 && checkName((char *)(addr + 9), num))
		{
			addr -= 12;
			U32V(addr | 0x40000000) = 0x3A30736D;
			U32V((addr + 4) | 0x40000000) = 0x2F4D542F;
			addr += 12;
			count ++;
		}
		addr += 4;
	}
	return count;
}

int (* sceKernelStartModule_ori)(SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option);

int sceKernelStartModule_patch(SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option)
{
	//int count = cxmbPatchMemory();
	SceModule *pMod;
	pMod = sceKernelFindModuleByUID(modid);
	int i;
	for(i = 0; i < pMod->nsegment; i++)
	{
		cxmbPatchMemory(pMod->segmentaddr[i], pMod->segmentsize[i], 13);
	}
	int ret = sceKernelStartModule_ori(modid, argsize, argp, status, option);
	return ret;
}

static int magic = 0, dfd = -1;

SceUID sceIoOpen_patch(const char * file, int flag, SceMode mode)
{
	if(cstrcmp((char *)((u32)file + 19), "music_plugin.prx") || cstrcmp((char *)((u32)file + 19), "game_plugin.prx") || cstrcmp((char *)((u32)file + 19), "opening_plugin.prx"))
	{
		magic = 1;
	}
	SceUID ret = sceIoOpen_ori(file, flag, mode);
	if(magic)
	{
		dfd = ret;
	}
	return ret;
}

int sceIoClose_patch(SceUID fd)
{
	if(magic && fd == dfd)
	{
		cxmbPatchMemory(0x09C80000, 0x0037FFFF, 3);
		magic = 0;
		dfd = -1;
	}
	int ret = sceIoClose_ori(fd);
	return ret;
}

static int checkFile(const char * path)
{
	int ret = sceIoOpen(path, PSP_O_RDONLY, 0777);
	if(ret >= 0)
	{
		sceIoClose(ret);
		return 1;
	}
	else return 0;
}

static const char * nesFont[] =
{
	"ms0:/TM/font/ltn0.pgf",
	"ms0:/TM/font/ltn4.pgf",
	"ms0:/TM/font/ltn10.pgf",
	"ms0:/TM/font/gb3s1518.bwfon",
	"ms0:/TM/font/kr0.pgf",
	"ms0:/TM/font/jpn0.pgf"
};

static void setFontPath(const char * font_path)
{
	struct RegParam reg;
	REGHANDLE h;
	memset(&reg, 0, sizeof(reg));
	reg.regtype = 1;
	reg.namelen = strlen("/system");
	reg.unk2 = 1;
	reg.unk3 = 1;
	strcpy(reg.name, "/system");
	if(sceRegOpenRegistry(&reg, 2, &h) == 0)
	{
		REGHANDLE hd;
		if(!sceRegOpenCategory(h, "/DATA/FONT", 2, &hd))
		{
			REGHANDLE hk;
			unsigned int type, size;

			if(!sceRegGetKeyInfo(hd, "path_name", &hk, &type, &size))
			{
				char path[size];
				memset(path, 0, size);
				strncpy(path, font_path, 12);
				if(!sceRegSetKeyValue(hd, "path_name", path, size))
				{
					sceRegFlushCategory(hd);
				}
			}
			sceRegCloseCategory(hd);
		}
		sceRegFlushRegistry(h);
		sceRegCloseRegistry(h);
	}
}

int (*sceRegInit_ori)(SceSize args, void *argp);

int sceRegInit_patch(SceSize args, void *argp)
{
	int ret = sceRegInit_ori(args, argp);
	if(font_patch)
		setFontPath("ms0:/TM/font");
	return ret;
}

int (*sceRegExit_ori)(SceSize args, void *argp);

int sceRegExit_patch(SceSize args, void *argp)
{
	if(font_patch)
		setFontPath("flash0:/font");
	return sceRegExit_ori(args, argp);
}

int power_callback(int unknown, int pwrflags, void *common)
{
    if(pwrflags & PSP_POWER_CB_POWER_SWITCH || pwrflags & PSP_POWER_CB_SUSPENDING)
	{
		if(font_patch)
			setFontPath("flash0:/font");
    }
	else if(pwrflags & PSP_POWER_CB_RESUMING)
	{
		if(font_patch)
			setFontPath("ms0:/TM/font");
    }
	else if(pwrflags & PSP_POWER_CB_RESUME_COMPLETE)
	{
		if(font_patch)
			setFontPath("ms0:/TM/font");
    }
	else if(pwrflags & PSP_POWER_CB_STANDBY)
	{
		if(font_patch)
			setFontPath("flash0:/font");
    }
    sceDisplayWaitVblankStart();

	return 0;
}

int CallbackThread(SceSize args, void *argp)
{
    int cbid;
    cbid = sceKernelCreateCallback("Power Callback", power_callback, NULL);
    scePowerRegisterCallback(0, cbid);
    sceKernelSleepThreadCB();

	return 0;
}

int SetupCallbacks()
{
    int thid = 0;
    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0)
	sceKernelStartThread(thid, 0, 0);
    return thid;
}

static void cxmbInit()
{
	if(checkFile(nesFont[0]) && checkFile(nesFont[1]) && checkFile(nesFont[2]) && checkFile(nesFont[3]) && checkFile(nesFont[4]) && checkFile(nesFont[5]))
	{
		font_patch = 1;
		setFontPath("ms0:/TM/font");
	}
	else setFontPath("flash0:/font");
	char path[64];
	int i;
	for(i = 0; i < 13; i++)
	{
		sprintf(path, "%s%s", "ms0:/TM/vsh/resource/", filename[i]);
		if(checkFile(path))
			patchfile[i] = 1;
		else patchfile[i] = 0;
	}
	
	SetupCallbacks();
}

int main_thread(SceSize args, void *argp)
{
	cxmbInit();
	sceIoOpen_ori = pspHookMethod("sceIOFileManager", "IoFileMgrForKernel", 0x109F50BC, sceIoOpen_patch, 1);
	pspHookMethod("sceIOFileManager", "IoFileMgrForUser", 0x109F50BC, sceIoOpen_patch, 0);
	sceIoClose_ori = pspHookMethod("sceIOFileManager", "IoFileMgrForKernel", 0x810C4BC3, sceIoClose_patch, 1);
	pspHookMethod("sceIOFileManager", "IoFileMgrForUser", 0x810C4BC3, sceIoClose_patch, 0);
	sceKernelStartModule_ori = pspHookMethod("sceModuleManager", "ModuleMgrForKernel", 0x50F0C1EC, sceKernelStartModule_patch, 1);
	pspHookMethod("sceModuleManager", "ModuleMgrForUser", 0x50F0C1EC, sceKernelStartModule_patch, 0);
	
	return sceKernelExitDeleteThread(0);
}

int module_start(SceSize args, void *argp)
{
	int thid1 = sceKernelCreateThread("cxmb_thread", main_thread, 10, 0x2000, 0, NULL);
	if(thid1 >= 0)
		sceKernelStartThread(thid1, args, argp);
	return 0;
}

int module_stop(SceSize args, void *argp)
{
	if(font_patch)
		setFontPath("flash0:/font");
	return 0;
}

int module_reboot_before(SceSize args, void *argp)
{
	if(font_patch)
		setFontPath("flash0:/font");
	return 0;
}

