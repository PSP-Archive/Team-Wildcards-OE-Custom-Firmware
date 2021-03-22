#include <pspsdk.h>
#include <pspkernel.h>
#include <pspreg.h>
#include <psprtc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "systemctrl.h"
#include "systemctrl_se.h"
#include "umd9660_driver.h"
#include "isofs_driver.h"

#include "virtualpbpmgr.h"
#include "main.h"

PSP_MODULE_INFO("VshControl", 0x1007, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

#define EBOOT_BIN "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"
#define BOOT_BIN  "disc0:/PSP_GAME/SYSDIR/BOOT.BIN"

void *oe_malloc(int size);
int oe_free(void *ptr);

SEConfig config; //0x40AC

int (* sceRegSetKeyValueReal)(REGHANDLE hd, const char *name, void *buf, SceSize size);

/* Note: the compiler has to be configured in makefile to make
   sizeof(wchar_t) = 2. */
wchar_t verinfo[] = CFW_VER_MAJOR L"." CFW_VER_MINOR L"*";

SceUID gamedfd = -1, //0x3D34
       game150dfd = -1, //0x3D30
       game3xxdfd = -1,
       isodfd = -1,
       over3xx = 0, //0x3EE4
       overiso = 0; //0x3EE0
SceUID paramsfo = -1;
int vpbpinited = 0,
    isoindex = 0, //0x3ED8
    cachechanged = 0;
VirtualPbp *cache = NULL;
int referenced[32];
APRS_EVENT previous;
u64 firsttick; //0x3EE8
int set;
VirtualPbp vpbp;

//0x00000000
void ClearCaches()
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
}

//0x0000001C
int LoadStartModule(char *module)
{
	SceUID mod = sceKernelLoadModule(module, 0, NULL);

	if (mod < 0)
		return mod;

	return sceKernelStartModule(mod, strlen(module)+1, module, NULL, NULL);
}

//0x0000008C
void KXploitString(char *str)
{
	if (str)
	{
		char *perc = strchr(str, '%');

		if (perc)
		{
			strcpy(perc, perc+1);
		}
	}
}

//0x000000C8
void Fix150Path(const char *file)
{
	char str[256];

	if (strstr(file, "ms0:/PSP/GAME/") == file)
	{
		strcpy(str, (char *)file);

		char *p = strstr(str, "__150");

		if (p)
		{
			strcpy((char *)file+13, "150/");
			strncpy((char *)file+17, str+14, p-(str+14));
			strcpy((char *)file+17+(p-(str+14)), p+5);
		}
	}
}

//0x0000016C
void Fix3xxPath(const char *file)
{
	char str[256];

	if (strstr(file, "ms0:/PSP/GAME/") == file)
	{
		strcpy(str, (char *)file);

		char *p = strstr(str, "__" CFW_VER_MAJOR CFW_VER_MINOR);

		if (p)
		{
			strcpy((char *)file+13, CFW_VER_MAJOR CFW_VER_MINOR "/");
			strncpy((char *)file+17, str+14, p-(str+14));
			strcpy((char *)file+17+(p-(str+14)), p+5);
		}
	}
}

//0x00000210
int CorruptIconPatch(char *name, int g150)
{
	char path[256];
	SceIoStat stat;

	if (g150)
	{
		sprintf(path, "ms0:/PSP/GAME150/%s%%/EBOOT.PBP", name);
	}

	else
	{
		sprintf(path, "ms0:/PSP/GAME/%s%%/EBOOT.PBP", name);
	}

	memset(&stat, 0, sizeof(stat));

	if (sceIoGetstat(path, &stat) >= 0)
	{
		strcpy(name, "__SCE"); // hide icon

		return 1;
	}

	return 0;
}

//0x000002A0
int GetIsoIndex(const char *file)
{
	char number[5];

	if (strstr(file, "ms0:/PSP/GAME/MMMMMISO") != file)
		return -1;

	char *p = strchr(file+17, '/');

	if (!p)
		return strtol(file+22, NULL, 10);

	memset(number, 0, 5);
	strncpy(number, file+22, p-(file+22));

	return strtol(number, NULL, 10);
}

u32 ERR_FILE_NOT_FOUND = 0x80000001;
u32 ERR_PBP_CANNOT_SEEK_HEADER = 0x80000002;
u32 ERR_PBP_CANNOT_SEEK_ELF_OFFSET = 0x80000003;
u32 ERR_PBP_CANNOT_READ_KMAGIC_OFFSET = 0x80000004;
u32 ERR_PBP_CANNOT_SEEK_KMAGIC = 0x80000005;
u32 ERR_PBP_UNKNOWN_ELF_TYPE = 0x80000006;
u32 ERR_ELF_CANNOT_SEEK_HEADER = 0x80000012;
u32 ERR_ELF_CANNOT_READ_KMAGIC_OFFSET = 0x80000014;
u32 ERR_ELF_CANNOT_SEEK_KMAGIC = 0x80000015;
u32 ERR_ELF_UNKNOWN_ELF_TYPE = 0x80000016;

u32 getProperError(char *file, u32 err)
{
	if(strstr(file, "UPDATE/EBOOT.BIN") || strstr(file, "EBOOT.BIN") || strstr(file, "GAME/UPDATE/EBOOT.PBP")) return 0xDADADADA;
	return err;
}

//0x00000324
void RebootVSHWithError(u32 error)
{
	struct SceKernelLoadExecVSHParam param;
	u32 vshmain_args[0x20/4];

	memset(&param, 0, sizeof(param));
	memset(vshmain_args, 0, sizeof(vshmain_args));

	vshmain_args[0/4] = 0x0400;
	vshmain_args[4/4] = 0x20;
	vshmain_args[0x14/4] = error;

	param.size = sizeof(param);
	param.args = 0x400;
	param.argp = vshmain_args;
	param.vshmain_args_size = 0x400;
	param.vshmain_args = vshmain_args;
	param.configfile = "/kd/pspbtcnf.txt";

	sctrlKernelExitVSH(&param);
}

//0x000003B0
int LoadExecVSHCommonPatched(int apitype, char *file, struct SceKernelLoadExecVSHParam *param, int unk2)
{
	int k1 = pspSdkSetK1(0);
	int rebooterror = 0;
	int index;
	u32 *mod;
	u32 text_addr;
	int reboot150 = 0;
	int (* LoadExecVSHCommon)(int apitype, char *file, struct SceKernelLoadExecVSHParam *param, int unk2);

	mod = (u32 *)sceKernelFindModuleByName("sceLoadExec");
	text_addr = *(mod+27);

#if CFW_VER == 340
#define LE_VSH_COMMON_ADDR    0x0EE8
#elif CFW_VER == 352
#define LE_VSH_COMMON_ADDR    0x0FB4
#endif

	LoadExecVSHCommon = (void *)(text_addr+ LE_VSH_COMMON_ADDR);

	SetUmdFile("");

	index = GetIsoIndex(file);

	if (index >= 0)
	{
		if (config.executebootbin)
		{
			strcpy(file, BOOT_BIN);
			param->args = strlen(BOOT_BIN)+1;
		}
		else
		{
			strcpy(file, EBOOT_BIN);
			param->args = strlen(EBOOT_BIN)+1;
		}

		param->argp = file;
		SetUmdFile(virtualpbp_getfilename(index));
		pspSdkSetK1(k1);
		return LoadExecVSHCommon(0x120, file, param, unk2);
	}

	//Fix150Path(file);
	//Fix150Path(param->argp);
	//Fix3xxPath(file);
	//Fix3xxPath(param->argp);



	// check for ELF vs. PRX; run ELF/kmode PRX in 1.50 and PRX in CFW kernel
	int bin_offset, kcheck_offset, fp;
	int kernelmagic;
	unsigned char filemagic[4], elfmagic[4];

	SceUID fi = sceIoOpen(file, PSP_O_RDONLY, 0777);

	if(strstr(file, "%") != NULL) // metadata-only PBP, from kxploit - read non-% EBOOT instead
	{
		strcpy(strstr(file, "%"), "/eboot.pbp\0");
		sceIoClose(fi);
		return LoadExecVSHCommonPatched(apitype, file, param, unk2);
	}

	if(!strstr(file, "disc0:/") && !strstr(file, "ms0:/PSP/GAME/MMMMMISO")) // it's not a UMD or an ISO
	{
		if(fi >= 0)
		{
			sceIoRead(fi, filemagic, sizeof(filemagic));
			if(!memcmp(filemagic, "\0PBP", 4)) // it's a PBP
			{
				fp = sceIoLseek(fi, 0x20, PSP_SEEK_SET);
				if(fp == 0x20)
				{
					sceIoLseek(fi, 0x20, PSP_SEEK_SET);
					sceIoRead(fi, &bin_offset, sizeof(int));

					fp = sceIoLseek(fi, bin_offset+0x10, PSP_SEEK_SET);
					if(fp == bin_offset+0x10)
					{
						sceIoRead(fi, elfmagic, 2*sizeof(u8));
						if(elfmagic[0]==2 && elfmagic[1]==0) // ELF
						{
							reboot150 = 1;
							KXploitString(file);
							KXploitString(param->argp);
						} else if(elfmagic[0]==0xA0 && elfmagic[1]==0xFF){ // PRX
							// check for kernel mode
							fp = sceIoLseek(fi, bin_offset+0x44, PSP_SEEK_SET);
							if(fp == bin_offset+0x44)
							{
								sceIoRead(fi, &kcheck_offset,sizeof(int));
								fp = sceIoLseek(fi, bin_offset+kcheck_offset-0x96, PSP_SEEK_SET);
								if(fp == bin_offset+kcheck_offset-0x96)
								{
									sceIoRead(fi, &kernelmagic, sizeof(int));
									if(kernelmagic == 0) // kernel mode
									{
										reboot150 = config.gamekernel150;
										KXploitString(file);
										KXploitString(param->argp);
									}
								} else rebooterror = getProperError(file, ERR_PBP_CANNOT_SEEK_KMAGIC);
							} else rebooterror = getProperError(file, ERR_PBP_CANNOT_READ_KMAGIC_OFFSET);
						} else {
							sceIoLseek(fi, bin_offset, PSP_SEEK_SET);
							sceIoRead(fi, elfmagic, 4*sizeof(u8));
							if(memcmp(elfmagic, "~PSP", 4)) // encrypted binary
								rebooterror = getProperError(file, ERR_PBP_UNKNOWN_ELF_TYPE);
						}
					} else rebooterror = getProperError(file, ERR_PBP_CANNOT_SEEK_ELF_OFFSET);
				} else rebooterror = getProperError(file, ERR_PBP_CANNOT_SEEK_HEADER);
			} else { // it's an ELF
				fp = sceIoLseek(fi, 0x10, PSP_SEEK_SET);
				if(fp == 0x10)
				{
					sceIoRead(fi, elfmagic, 2*sizeof(u8));
					if(elfmagic[0]==2 && elfmagic[1]==0)
					{
						reboot150 = 1;
						KXploitString(file);
						KXploitString(param->argp);
					} else if(elfmagic[0]==0xA0 && elfmagic[1]==0xFF){
					// check for kernel mode
						fp = sceIoLseek(fi, 0x44, PSP_SEEK_SET);
						if(fp == 0x44)
						{
							sceIoRead(fi, &kcheck_offset,sizeof(int));
							fp = sceIoLseek(fi, kcheck_offset-0x96, PSP_SEEK_SET);
							if(fp == kcheck_offset-0x96)
							{
								sceIoRead(fi, &kernelmagic, sizeof(int));
								if(kernelmagic == 0) // kernel mode
								{
									reboot150 = 1;
									KXploitString(file);
									KXploitString(param->argp);
								}
							} else rebooterror = getProperError(file, ERR_ELF_CANNOT_SEEK_KMAGIC);
						} else rebooterror = getProperError(file, ERR_ELF_CANNOT_READ_KMAGIC_OFFSET);
					} else rebooterror = getProperError(file, ERR_ELF_UNKNOWN_ELF_TYPE);
				} else rebooterror = getProperError(file, ERR_ELF_CANNOT_SEEK_HEADER);
			}
			sceIoClose(fi);
		} else rebooterror = getProperError(file, ERR_FILE_NOT_FOUND);
	}

	// the code above makes the whole GAME150/GAMECFW thing obsolete :)
	/*if (strstr(file, "ms0:/PSP/GAME150/") == file)
	{
		reboot150 = 1;
		KXploitString(file);
		KXploitString(param->argp);
	}

	else if (strstr(file, "ms0:/PSP/GAME/") == file)
	{*/
		if (config.gamekernel150)
		{
			reboot150 = 1;
			KXploitString(file);
			KXploitString(param->argp);
		}
	//}

	if (strstr(file, "UPDATE/EBOOT.BIN"))
	{
		rebooterror = 0xDADADADA;
	}

	else if (strstr(file, "EBOOT.BIN"))
	{
		if (config.executebootbin)
		{
			strcpy(file, BOOT_BIN);
			param->argp = file;
		}
	}

	else if (strstr(file, "GAME/UPDATE/EBOOT.PBP"))
	{
		SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
		u32 header[10];
		char buf[8];

		if (fd >= 0)
		{
			sceIoRead(fd, &header, sizeof(header));
			sceIoLseek(fd, header[8]+10, PSP_SEEK_SET);
			sceIoRead(fd, buf, 8);

			if (strcmp(buf, "updater") == 0)
			{
				rebooterror = 0xDADADADA;
			}
			else
			{
				sceIoLseek(fd, 0x4DE, PSP_SEEK_SET);
				sceIoRead(fd, buf, 5);

				if (strcmp(buf, "X.YZ") == 0)
				{
					rebooterror = 0xDADADADA;
				}
			}

			sceIoClose(fd);
		}
	}

	if (rebooterror)
	{
		RebootVSHWithError(rebooterror);
	}

	param->args = strlen(param->argp) + 1; // update length

	if (reboot150 == 1)
	{
		//reboot = (u8 *)malloc(51000);

		//ReadFile("flash0:/kd/reboot150.gz", reboot, 51000);
		LoadStartModule("flash0:/kd/reboot150.prx");
	}

	pspSdkSetK1(k1);

	return LoadExecVSHCommon(apitype, file, param, unk2);
}

//0x000006A8
SceUID sceIoDopenPatched(const char *dirname)
{
	int res, g150 = 0, index;
	int k1 = pspSdkSetK1(0);

	Fix150Path(dirname);
	Fix3xxPath(dirname);

	index = GetIsoIndex(dirname);
	if (index >= 0)
	{
		int res = virtualpbp_open(index);

		pspSdkSetK1(k1);
		return res;
	}

	if (strcmp(dirname, "ms0:/PSP/GAME") == 0)
	{
		g150 = 1;
	}

	res = sceIoDopen(dirname);

	if (g150)
	{
		gamedfd = res;
		game150dfd = sceIoDopen("ms0:/PSP/GAME150");
		over3xx = 0;
		overiso = 0;
	}

	pspSdkSetK1(k1);

	return res;
}

//0x00000780
void ApplyNamePatch(SceIoDirent *dir, char *patch)
{
	if (dir->d_name[0] != '.')
	{
		int patchname = 1;

		if (config.hidecorrupt)
		{
			if (CorruptIconPatch(dir->d_name, 1))
				patchname = 0;
		}

		if (patchname)
		{
			strcat(dir->d_name, patch);
		}
	}
}

//0x000007F8
void ApplyIsoNamePatch(SceIoDirent *dir)
{
	if (dir->d_name[0] != '.')
	{
		memset(dir->d_name, 0, 256);
		sprintf(dir->d_name, "MMMMMISO%d", isoindex++);
	}
}

//0x0000086C
int ReadCache()
{
	SceUID fd;
	int i;

	if (!cache)
	{
		cache = (VirtualPbp *)oe_malloc(32*sizeof(VirtualPbp));
	}

	memset(cache, 0, sizeof(VirtualPbp)*32);
	memset(referenced, 0, sizeof(referenced));

	for (i = 0; i < 0x10; i++)
	{
		fd = sceIoOpen("ms0:/PSP/SYSTEM/isocache.bin", PSP_O_RDONLY, 0777);

		if (fd >= 0)
			break;
	}

	if (i == 0x10)
		return -1;

	sceIoRead(fd, cache, sizeof(VirtualPbp)*32);
	sceIoClose(fd);

	return 0;
}

//0x00000928
int SaveCache()
{
	SceUID fd;
	int i;

	if (!cache)
		return -1;

	for (i = 0; i < 32; i++)
	{
		if (cache[i].isofile[0] != 0 && !referenced[i])
		{
			cachechanged = 1;
			memset(&cache[i], 0, sizeof(VirtualPbp));
		}
	}

	if (!cachechanged)
		return 0;

	cachechanged = 0;

	for (i = 0; i < 0x10; i++)
	{
		//sceIoMkdir("ms0:/PSP/SYSTEM", 0777);

		fd = sceIoOpen("ms0:/PSP/SYSTEM/isocache.bin", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

		if (fd >= 0)
			break;
	}

	if (i == 0x10)
		return -1;

	sceIoWrite(fd, cache, sizeof(VirtualPbp)*32);
	sceIoClose(fd);

	return 0;
}

//0x00000A18
int IsCached(char *isofile, ScePspDateTime *mtime, VirtualPbp *res)
{
	int i;

	for (i = 0; i < 32; i++)
	{
		if (cache[i].isofile[0] != 0)
		{
			if (strcmp(cache[i].isofile, isofile) == 0)
			{
				if (memcmp(mtime, &cache[i].mtime, sizeof(ScePspDateTime)) == 0)
				{
					memcpy(res, &cache[i], sizeof(VirtualPbp));
					referenced[i] = 1;
					return 1;
				}
			}
		}
	}

	return 0;
}

//0x00000B14
int Cache(VirtualPbp *pbp)
{
	int i;

	for (i = 0; i < 32; i++)
	{
		if (cache[i].isofile[0] == 0)
		{
			referenced[i] = 1;
			memcpy(&cache[i], pbp, sizeof(VirtualPbp));
			cachechanged = 1;
			return 1;
		}
	}

	return 0;
}

//0x00000BA0
int sceIoDreadPatched(SceUID fd, SceIoDirent *dir)
{
	int res;
	u32 k1 = pspSdkSetK1(0);

	if (vpbpinited)
	{
		res = virtualpbp_dread(fd, dir);
		if (res >= 0)
		{
			pspSdkSetK1(k1);
			return res;
		}
	}

	if (fd >= 0)
	{
		if (fd == gamedfd)
		{
			if (game150dfd >= 0)
			{
				if ((res = sceIoDread(game150dfd, dir)) > 0)
				{
					ApplyNamePatch(dir, "__150");
					pspSdkSetK1(k1);
					return res;
				}
				else
				{
					sceIoDclose(game150dfd);
					game150dfd = -1;
				}
			}

			if (game150dfd < 0 && game3xxdfd < 0 && isodfd < 0 && !over3xx)
			{
				game3xxdfd = sceIoDopen("ms0:/PSP/GAME" CFW_VER_MAJOR CFW_VER_MINOR);
				if (game3xxdfd < 0)
				{
					over3xx = 1;
				}
			}
			
			if (game3xxdfd >= 0)
			{
				if ((res = sceIoDread(game3xxdfd, dir)) > 0)
				{
					ApplyNamePatch(dir, "__" CFW_VER_MAJOR CFW_VER_MINOR);
					pspSdkSetK1(k1);
					return res;
				}
				else
				{
					sceIoDclose(game3xxdfd);
					game3xxdfd = -1;
					over3xx = 1;
				}
			}

			if (game150dfd < 0 && game3xxdfd < 0 && isodfd < 0 && !overiso)
			{
				isodfd = sceIoDopen("ms0:/ISO");

				if (isodfd >= 0)
				{
					if (!vpbpinited)
					{
						virtualpbp_init();
						vpbpinited = 1;							
					}
					else
					{
						virtualpbp_reset();						
					}

					ReadCache();
					isoindex = 0;
				}
				else
				{
					overiso = 1;
				}
			}

			if (isodfd >= 0)
			{
				if ((res = sceIoDread(isodfd, dir)) > 0)
				{
					char fullpath[128];
					int  res2 = -1;
					int  docache;

					if (!FIO_S_ISDIR(dir->d_stat.st_mode))
					{
						strcpy(fullpath, "ms0:/ISO/");
						strcat(fullpath, dir->d_name);

						if (IsCached(fullpath, &dir->d_stat.st_mtime, &vpbp))
						{
							res2 = virtualpbp_fastadd(&vpbp);
							docache = 0;
						}
						else
						{
							res2 = virtualpbp_add(fullpath, &dir->d_stat.st_mtime, &vpbp);
							docache = 1;
						}
						
						if (res2 >= 0)
						{
							ApplyIsoNamePatch(dir);

							// Fake the entry from file to directory
							dir->d_stat.st_mode = 0x11FF;
							dir->d_stat.st_attr = 0x0010;
							dir->d_stat.st_size = 0;	
							
							// Change the modifcation time to creation time
							memcpy(&dir->d_stat.st_mtime, &dir->d_stat.st_ctime, sizeof(ScePspDateTime));

							if (docache)
							{
								Cache(&vpbp);									
							}
						}
					}
					
					pspSdkSetK1(k1);
					return res;
				}

				else
				{
					sceIoDclose(isodfd);
					isodfd = -1;
					overiso = 1;
				}
			}			
		}
	}

	res = sceIoDread(fd, dir);

	if (res > 0)
	{
		if (config.hidecorrupt)
			CorruptIconPatch(dir->d_name, 0);
	}

	pspSdkSetK1(k1);
	return res;
}

//0x00000EF4
int sceIoDclosePatched(SceUID fd)
{
	u32 k1 = pspSdkSetK1(0);
	int res;

	if (vpbpinited)
	{
		res = virtualpbp_close(fd);
		if (res >= 0)
		{
			pspSdkSetK1(k1);
			return res;
		}
	}

	if (fd == gamedfd)
	{
		gamedfd = -1;
		over3xx = 0;
		overiso = 0;
		SaveCache();
	}

	pspSdkSetK1(k1);
	return sceIoDclose(fd);
}

//0x00000FAC
SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode)
{
	u32 k1 = pspSdkSetK1(0);
	int index;

	Fix150Path(file);
	Fix3xxPath(file);

	//Kprintf("opening file; ra = %08X %s\n", sceKernelGetSyscallRA(), file);

	index = GetIsoIndex(file);
	if (index >= 0)
	{
		int res = virtualpbp_open(index);

		pspSdkSetK1(k1);
		return res;
	}

	if (strstr(file, "disc0:/PSP_GAME/PARAM.SFO"))
	{
		pspSdkSetK1(k1);
		paramsfo = sceIoOpen(file, flags, mode);
		return paramsfo;
	}

	pspSdkSetK1(k1);

	return sceIoOpen(file, flags, mode);
}

//0x000010A8
int sceIoClosePatched(SceUID fd)
{
	u32 k1 = pspSdkSetK1(0);
	int res = -1;

	if (vpbpinited)
	{
		res = virtualpbp_close(fd);
	}

	if (fd == paramsfo)
	{
		paramsfo = -1;
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoClose(fd);

	return res;
}

//0x00001148
int sceIoReadPatched(SceUID fd, void *data, SceSize size)
{
	u32 k1 = pspSdkSetK1(0);	
	int res = -1;
	
	if (vpbpinited)
	{
		res = virtualpbp_read(fd, data, size);		
	}

	if (fd == paramsfo)
	{
		int i;

		pspSdkSetK1(k1);		
		res = sceIoRead(fd, data, size);
		pspSdkSetK1(0);

		if (res > 4)
		{
			for (i = 0; i < res-4; i++)
			{
				if (memcmp(data+i, "3.", 2) == 0)
				{
					if (strlen(data+i) == 4)
					{
						memcpy(data+i, "3.40", 4);
						break;
					}
				}
			}
		}

		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoRead(fd, data, size);

	return res;
}

//0x000012B4
SceOff sceIoLseekPatched(SceUID fd, SceOff offset, int whence)
{
	u32 k1 = pspSdkSetK1(0);
	int res = -1;

	if (vpbpinited)
	{
		res = virtualpbp_lseek(fd, offset, whence);
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoLseek(fd, offset, whence);

	return res;
}

//0x00001388
int sceIoLseek32Patched(SceUID fd, int offset, int whence)
{
	u32 k1 = pspSdkSetK1(0);
	int res = -1;

	if (vpbpinited)
	{
		res = virtualpbp_lseek(fd, offset, whence);
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoLseek32(fd, offset, whence);

	return res;
}

//0x00001440
int sceIoGetstatPatched(const char *file, SceIoStat *stat)
{
	u32 k1 = pspSdkSetK1(0);
	int index;

	Fix150Path(file);
	Fix3xxPath(file);

	index = GetIsoIndex(file);
	if (index >= 0)
	{
		int res = virtualpbp_getstat(index, stat);

		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);

	return sceIoGetstat(file, stat);
}

//0x000014E4
int sceIoChstatPatched(const char *file, SceIoStat *stat, int bits)
{
	u32 k1 = pspSdkSetK1(0);
	int index;

	Fix150Path(file);
	Fix3xxPath(file);

	index = GetIsoIndex(file);
	if (index >= 0)
	{
		int res = virtualpbp_chstat(index, stat, bits);

		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);

	return sceIoChstat(file, stat, bits);
}

//0x000015A0
int sceIoRemovePatched(const char *file)
{
	u32 k1 = pspSdkSetK1(0);
	int index;

	Fix150Path(file);
	Fix3xxPath(file);

	index = GetIsoIndex(file);
	if (index >= 0)
	{
		int res = virtualpbp_remove(index);
		
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);

	return sceIoRemove(file);
}

//0x0000162C
int sceIoRmdirPatched(const char *path)
{
	u32 k1 = pspSdkSetK1(0);
	int index;

	Fix150Path(path);
	Fix3xxPath(path);

	index = GetIsoIndex(path);
	if (index >= 0)
	{
		int res = virtualpbp_rmdir(index);

		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);

	return sceIoRmdir(path);
}

//0x000016B8
int sceIoMkdirPatched(const char *dir, SceMode mode)
{
	int k1 = pspSdkSetK1(0);

	if (strcmp(dir, "ms0:/PSP/GAME") == 0 || strcmp(dir, "ms0:/PSP/GAME/") == 0)
	{
		sceIoMkdir("ms0:/PSP/GAME150", mode);
		sceIoMkdir("ms0:/PSP/GAME" CFW_VER_MAJOR CFW_VER_MINOR, mode);
		sceIoMkdir("ms0:/ISO", mode);
	}

	pspSdkSetK1(k1);
	return sceIoMkdir(dir, mode);
}

//0x00001768
int sceRegSetKeyValuePatched(REGHANDLE hd, const char *name, void *buf, SceSize size)
{
	int k1 = pspSdkSetK1(0);

	if (strcmp(name, "language") == 0)
	{
		int lang = *(int *)buf;

		if (lang == 0x09 /* korean */ ||
		    lang == 0x0A /* chinese */ ||
			lang == 0x0B) /* the other chinese */
		{
			RebootVSHWithError(0x98765432);
			// We shouldn't get here, but if that, then let's loop forever
			while (1);
		}
	}
	
	pspSdkSetK1(k1);

	return sceRegSetKeyValue(hd, name, buf, size);
}

//0x00001814 - patch targets updated
void IoPatches()
{
	u32 *mod, text_addr;

	mod = (u32 *)sceKernelFindModuleByName("sceIOFileManager");

	if (mod)
	{
		text_addr = *(mod+27);

		// Patcth IoFileMgrForUser nids
#if CFW_VER == 340
#define SCEIODOPEN_ADDR   0x143C
#define SCEIODREAD_ADDR   0x1588
#define SCEIODCLOSE_ADDR  0x1638
#define SCEIOOPEN_ADDR    0x3B10
#define SCEIOCLOSE_ADDR   0x3AD0
#define SCEIOREAD_ADDR    0x3C28
#define SCEIOLSEEK_ADDR   0x3C98
#define SCEIOLSEEK32_ADDR 0x3CD0
#define SCEIOGETSTAT_ADDR 0x3DC4
#define SCEIOCHSTAT_ADDR  0x3DE4
#define SCEIOREMOVE_ADDR  0x16D8
#define SCEIORMDIR_ADDR   0x3D84
#define SCEIOMKDIR_ADDR   0x3D68
#define SCEIORENAME_ADDR  0x1810
#elif CFW_VER == 352
#define SCEIODOPEN_ADDR   0x142C
#define SCEIODREAD_ADDR   0x1578
#define SCEIODCLOSE_ADDR  0x1628
#define SCEIOOPEN_ADDR    0x3BFC
#define SCEIOCLOSE_ADDR   0x3BBC
#define SCEIOREAD_ADDR    0x3D14
#define SCEIOLSEEK_ADDR   0x3D84
#define SCEIOLSEEK32_ADDR 0x3DBC
#define SCEIOGETSTAT_ADDR 0x3EB0
#define SCEIOCHSTAT_ADDR  0x3ED0
#define SCEIOREMOVE_ADDR  0x16C8
#define SCEIORMDIR_ADDR   0x3E70
#define SCEIOMKDIR_ADDR   0x3E54
#define SCEIORENAME_ADDR  0x17EC
#endif

		PatchSyscall(text_addr+ SCEIODOPEN_ADDR, sceIoDopenPatched);
		PatchSyscall(text_addr+ SCEIODREAD_ADDR, sceIoDreadPatched);
		PatchSyscall(text_addr+ SCEIODCLOSE_ADDR, sceIoDclosePatched);
		PatchSyscall(text_addr+ SCEIOOPEN_ADDR, sceIoOpenPatched);
		PatchSyscall(text_addr+ SCEIOCLOSE_ADDR, sceIoClosePatched);
		PatchSyscall(text_addr+ SCEIOREAD_ADDR, sceIoReadPatched);
		PatchSyscall(text_addr+ SCEIOLSEEK_ADDR, sceIoLseekPatched);
		PatchSyscall(text_addr+ SCEIOLSEEK32_ADDR, sceIoLseek32Patched);
		PatchSyscall(text_addr+ SCEIOGETSTAT_ADDR, sceIoGetstatPatched);
		PatchSyscall(text_addr+ SCEIOCHSTAT_ADDR, sceIoChstatPatched);
		PatchSyscall(text_addr+ SCEIOREMOVE_ADDR, sceIoRemovePatched);
		PatchSyscall(text_addr+ SCEIORMDIR_ADDR, sceIoRmdirPatched);
		PatchSyscall(text_addr+ SCEIOMKDIR_ADDR, sceIoMkdirPatched);
		//PatchSyscall(text_addr+ SCEIORENAME_ADDR, sceIoRenamePatched);
	}

	mod = (u32 *)sceKernelFindModuleByName("sceMSFAT_Driver");
	if (mod)
	{
		text_addr = *(mod+27);

		// Make an odd patch to allow sceIoGetstat be called correctly in kernel mode
#if CFW_VER == 340
#define IOGETSTAT_PATCH_ADDR   0x22CC
#elif CFW_VER == 352
#define IOGETSTAT_PATCH_ADDR   0x22D8
#endif

		_sw(NOP, text_addr+ IOGETSTAT_PATCH_ADDR);
	}
}

//0x00001930 - patch targets updated
void PatchVshMain(u8 *buf)
{
	int intr = sceKernelCpuSuspendIntr();

	u32 text_addr = (u32)buf+0xA0;

	// Allow old sfo's.
#if CFW_VER == 340
#define VSHMAIN_OLDSFO_ADDR1 0xE720
#define VSHMAIN_OLDSFO_ADDR2 0xE728
#elif CFW_VER == 352
#define VSHMAIN_OLDSFO_ADDR1 0xE728
#define VSHMAIN_OLDSFO_ADDR2 0xE730
#endif

	_sw(NOP, (u32)(text_addr+ VSHMAIN_OLDSFO_ADDR1));
	_sw(NOP, (u32)(text_addr+ VSHMAIN_OLDSFO_ADDR2));

	u32 x = FindProc("sceRegistry_Service", "sceReg", 0x17768E14);

	PatchSyscall(x, sceRegSetKeyValuePatched);

	IoPatches();

	sceKernelCpuResumeIntr(intr);
	ClearCaches();
}

//0x000019B4 - patch targets updated
void PatchSysconfPlugin(u8 *buf)
{
	u32 addrlow, addrhigh;
	int intr = sceKernelCpuSuspendIntr();

	u32 text_addr = (u32)(buf+0xA0);

#if CFW_VER == 340
#define SP_MODULE_NAME  0x1A420
#define SP_PATCH_ADDR   0xD574
#elif CFW_VER == 352
#define SP_MODULE_NAME  0x1AB10
#define SP_PATCH_ADDR   0xDAF0
#endif

	memcpy((void *)(text_addr+ SP_MODULE_NAME), verinfo, sizeof(verinfo));

	addrhigh = (text_addr+ SP_MODULE_NAME -0xA0) >> 16;
	addrlow = (text_addr+ SP_MODULE_NAME -0xA0) & 0xFFFF;

	// lui v0, addrhigh
	_sw(0x3c020000 | addrhigh, text_addr+ SP_PATCH_ADDR);
	// ori v0, v0, addrlow
	_sw(0x34420000 | addrlow, text_addr+ SP_PATCH_ADDR + 4);

	sceKernelCpuResumeIntr(intr);
	ClearCaches();
}

//0x00001A70 - patch targets updated
void PatchGamePlugin(u8 *buf)
{
#if CFW_VER == 340
#define GP_PATCH_ADDR   0xD2F0
#elif CFW_VER == 352
#define GP_PATCH_ADDR   0xFDCC
#endif

	u32 text_addr = (u32)(buf+0xA0);
	_sw(0x1000fff9, text_addr + GP_PATCH_ADDR);
	_sw(0x24040000, text_addr + GP_PATCH_ADDR + 4);
	ClearCaches();
}

//0x00001B3C - patch targets NOT updated
void PatchMsVideoMainPlugin(u8 *buf)
{
	int intr = sceKernelCpuSuspendIntr();
	u32 text_addr = (u32)(buf+0xA0);

	// Patch resolution limit to 130560 pixels (480x272)
        // Allow play avc <= 480*272
#if CFW_VER == 340
#define MSVM_RES_PATCH1_ADDR 0x343CC
#define MSVM_RES_PATCH2_ADDR 0x36324
#define MSVM_RES_PATCH3_ADDR 0x36398
#define MSVM_RES_PATCH4_ADDR 0x3644c
#define MSVM_RES_PATCH5_ADDR 0x3ced8
#define MSVM_RES_PATCH6_ADDR 0x3cfec
#define MSVM_RES_PATCH7_ADDR 0x49890
#define MSVM_RES_PATCH8_ADDR 0x499d4
#define MSVM_RES_PATCH9_ADDR 0x4d2b0
#define MSVM_BR_PATCH1_ADDR  0x362C0
#define MSVM_BR_PATCH2_ADDR  0x3634C
#elif CFW_VER == 352
#define MSVM_RES_PATCH1_ADDR 0x32568
#define MSVM_RES_PATCH2_ADDR 0x325D4
#define MSVM_RES_PATCH3_ADDR 0x345B4
#define MSVM_RES_PATCH4_ADDR 0x34628
#define MSVM_RES_PATCH5_ADDR 0x346E8
#define MSVM_RES_PATCH6_ADDR 0x3A694
#define MSVM_RES_PATCH7_ADDR 0x3A7A8
#define MSVM_RES_PATCH8_ADDR 0x488E0
#define MSVM_BR_PATCH1_ADDR  0x34550
#define MSVM_BR_PATCH2_ADDR  0x345DC
#endif

        //ori $v0, $v0, 0x2C00 -> ori $v0, $v0, 0xFE00
	_sh(0xfe00, text_addr+ MSVM_RES_PATCH1_ADDR);
	_sh(0xfe00, text_addr+ MSVM_RES_PATCH2_ADDR);
	_sh(0xfe00, text_addr+ MSVM_RES_PATCH3_ADDR);
	_sh(0xfe00, text_addr+ MSVM_RES_PATCH4_ADDR);
	_sh(0xfe00, text_addr+ MSVM_RES_PATCH5_ADDR);
	_sh(0xfe00, text_addr+ MSVM_RES_PATCH6_ADDR);
	_sh(0xfe00, text_addr+ MSVM_RES_PATCH7_ADDR);
	_sh(0xfe00, text_addr+ MSVM_RES_PATCH8_ADDR);
#ifdef MSVM_RES_PATCH9_ADDR
	_sh(0xfe00, text_addr+ MSVM_RES_PATCH9_ADDR);
#endif

	// Patch bitrate limit	(increase to 16384+2)
	//sltiu $v0, $v0, 0x303 -> sltiu $v0, $v0, 0x4003
	_sh(0x4003, text_addr+ MSVM_BR_PATCH1_ADDR);
	_sh(0x4003, text_addr+ MSVM_BR_PATCH2_ADDR);

	sceKernelCpuResumeIntr(intr);

	ClearCaches();
}

//0x00001C04
int OnModuleRelocated(char *modname, u8 *modbuf)
{
	if (config.vshcpuspeed != 0 && !set)
	{
		u64 curtick;
		u32 t;

		sceRtcGetCurrentTick(&curtick);
		curtick -= firsttick;
		t = (u32)curtick;

		if (t >= (10*1000*1000))
		{
			set = 1;
			SetSpeed(config.vshcpuspeed, config.vshbusspeed);
		}
	}

	if (strcmp(modname, "vsh_module") == 0)
	{
		PatchVshMain(modbuf);
	}
	else if (strcmp(modname, "sysconf_plugin_module") == 0)
	{
		PatchSysconfPlugin(modbuf);
	}

	else if (strcmp(modname, "msvideo_main_plugin_module") == 0)
	{
		PatchMsVideoMainPlugin(modbuf);
	}

	else if (strcmp(modname, "game_plugin_module") == 0)
	{
		PatchGamePlugin(modbuf);
	}

	if (!previous)
		return 0;

	return previous(modname, modbuf);
}

//0x00001AA0
int module_start(SceSize args, void *argp)
{
	u32 *mod = (u32 *)sceKernelFindModuleByName("sceLoadExec");
	u32 text_addr = *(mod+27);

#if CFW_VER == 340
#define LE_VSH_COMMON_CALL1_ADDR  0x0930
#define LE_VSH_COMMON_CALL2_ADDR  0x0958
#define LE_VSH_COMMON_CALL3_ADDR  0x0A80
#define LE_VSH_COMMON_CALL4_ADDR  0x0AA8
#elif CFW_VER == 352
#define LE_VSH_COMMON_CALL1_ADDR  0x09D4
#define LE_VSH_COMMON_CALL2_ADDR  0x09FC
#define LE_VSH_COMMON_CALL3_ADDR  0x0B4C
#define LE_VSH_COMMON_CALL4_ADDR  0x0B74
#endif

	MAKE_CALL(text_addr+ LE_VSH_COMMON_CALL1_ADDR, LoadExecVSHCommonPatched); //sceKernelLoadExecVSHDisc
	MAKE_CALL(text_addr+ LE_VSH_COMMON_CALL2_ADDR, LoadExecVSHCommonPatched); //sceKernelLoadExecVSHDiscUpdater
	MAKE_CALL(text_addr+ LE_VSH_COMMON_CALL3_ADDR, LoadExecVSHCommonPatched); //sceKernelLoadExecVSHMs1
	MAKE_CALL(text_addr+ LE_VSH_COMMON_CALL4_ADDR, LoadExecVSHCommonPatched); //sceKernelLoadExecVSHMs2

	sctrlSEGetConfig(&config);

	if (config.vshcpuspeed != 0)
	{
		sceRtcGetCurrentTick(&firsttick);
	}

	previous = sctrlHENSetOnApplyPspRelSectionEvent(OnModuleRelocated);

	return 0;
}
