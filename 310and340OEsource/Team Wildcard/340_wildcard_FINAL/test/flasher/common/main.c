#include "common.h"
#include <string.h>

PSP_MODULE_INFO("MULTI_CFW_FLASHER", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

int Encrypt(u32 *buf, int size);
int GenerateSigCheck(u8 *buf);
u32 FindProc(const char* szMod, const char* szLib, u32 nid);
void getcheck(unsigned char *, int, unsigned char *);
int ByPass();
void remove_flasher_files_and_exit();
int main(int, char **);
extern void check_dxar();
extern void write_cfw();

typedef int (*PROC_MANGLE)(void* r4, u32 r5, void* r6, u32 r7, u32 r8);
PROC_MANGLE g_mangleProc;
    // secret access to hardware decryption AKA "semaphore_2"
    // r8=opcode: 7=>block cypher/scramble (many uses),
        // 11=>SHA1, 1=>magic decode of real PRX code

u8 check_keys0[0x10] =
{
	0x71, 0xF6, 0xA8, 0x31, 0x1E, 0xE0, 0xFF, 0x1E,
	0x50, 0xBA, 0x6C, 0xD2, 0x98, 0x2D, 0xD6, 0x2D
}; 

u8 check_keys1[0x10] =
{
	0xAA, 0x85, 0x4D, 0xB0, 0xFF, 0xCA, 0x47, 0xEB,
	0x38, 0x7F, 0xD7, 0xE4, 0x3D, 0x62, 0xB0, 0x10
};

int Encrypt(u32 *buf, int size)
{
	buf[0] = 4;
	buf[1] = buf[2] = 0;
	buf[3] = 0x100;
	buf[4] = size;

	/* Note: this encryption returns different data in each psp,
	   But it always returns the same in a specific psp (even if it has two nands) */
	if (g_mangleProc(buf, size+0x14, buf, size+0x14, 5) < 0)
		return -1;

	return 0;
}

int GenerateSigCheck(u8 *buf)
{
	u8 enc[0xD0+0x14];
	int iXOR, res;

	memcpy(enc+0x14, buf+0x110, 0x40);
	memcpy(enc+0x14+0x40, buf+0x80, 0x90);
	
	for (iXOR = 0; iXOR < 0xD0; iXOR++)
	{
		enc[0x14+iXOR] ^= check_keys0[iXOR&0xF]; 
	}

	if ((res = Encrypt((u32 *)enc, 0xD0)) < 0)
	{
		printf("Encrypt failed.\n");
		return res;
	}

	for (iXOR = 0; iXOR < 0xD0; iXOR++)
	{
		enc[0x14+iXOR] ^= check_keys1[iXOR&0xF];
	}

	memcpy(buf+0x80, enc+0x14, 0xD0);
	return 0;
}

/* New FindProc based on tyranid's psplink code. PspPet one doesn't work
   well with 2.7X+ sysmem.prx */
u32 FindProc(const char* szMod, const char* szLib, u32 nid)
{
	struct SceLibraryEntryTable *entry;
	SceModule *pMod;
	char *entTab;
	int entLen;

	pMod = sceKernelFindModuleByName(szMod);

	if (!pMod)
	{
		printf("Cannot find module %s\n", szMod);
		return 0;
	}
	
	int i = 0;

	entTab = (char *)pMod->ent_top;
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
			vars = (unsigned int *)(entry->entrytable);

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

	printf("Funtion not found.\n");
	return 0;
}

void getcheck(unsigned char *block, int len, unsigned char *check)
{
	SceKernelUtilsSha1Context ctx;

	sceKernelUtilsSha1BlockInit(&ctx);
	sceKernelUtilsSha1BlockUpdate(&ctx, block, len);
	sceKernelUtilsSha1BlockResult(&ctx, check);
}

char *getVersion(int x)
{
	static char ver[5], hex[9];
	sprintf(hex, "%08X", x);
	ver[0]=hex[1];
	ver[1]='.';
	ver[2]=hex[3];
	ver[3]=hex[5];
	ver[4]='\0';
	return ver;
}

int ByPass()
{
	SceCtrlData pad;

	sceCtrlReadBufferPositive(&pad, 1);

	if ((pad.Buttons & PSP_CTRL_TRIANGLE) && (pad.Buttons & PSP_CTRL_LTRIGGER))
			return 1;

	return 0;
}

void remove_flasher_files_and_exit()
{
#ifdef AIO
	sceIoRemove("ms0:/flasher.elf");
#endif
	sceKernelExitGame();
}


int main(int argc, char **argv) 
{
	g_mangleProc = (PROC_MANGLE)FindProc("sceMemlmd", "semaphore", 0x4c537c72);

	pspDebugScreenInit();
	pspDebugScreenSetTextColor(0x005435D0);
    pspDebugScreenClear();
    //setup Pad
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(0);

#ifndef USE_MS0
	if(sceKernelDevkitVersion() != 0x01050001)
	{
		printf("This firmware can only be installed from firmware 1.50\nor a custom firmware. Press X to quit.");
		pause_for_cross();
		remove_flasher_files_and_exit();
		return -1;
	}
	
	if (scePowerIsBatteryExist())
	{
		if(scePowerGetBatteryLifePercent() < 75)
		{
			if(!ByPass())
			{
				printf("The battery level must be above 75%% to install.\nPress X to quit.");
				pause_for_cross();
				remove_flasher_files_and_exit();
				return -1;
			}
		}
	} else {
		printf("The battery must be connected to install.\nPress X to quit.");
		pause_for_cross();
		remove_flasher_files_and_exit();
		return -1;
	}

	printf("Current PSP Version: %s\nPress X to begin, or Home to quit.\n", getVersion(sceKernelDevkitVersion()));
	pause_for_cross();
#endif
	check_dxar();
	write_cfw();
#ifdef USE_MS0
	printf("Unpack complete! Press X to quit.");
	pause_for_cross();
	remove_flasher_files_and_exit();
#else
	printf("\n\nInstallation complete!\nTo begin using your new firmware, perform a manual reboot :)");
	sceKernelSleepThread();
#endif
	return 0;
}
