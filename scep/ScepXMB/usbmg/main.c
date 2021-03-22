
#include <pspkernel.h>
#include <pspsdk.h>
#include <pspusb.h>
#include <pspsysmem.h>

#include <usbhostfs.h>
#include <systemctrl_se.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PSP_MODULE_INFO("SCX_MG", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

#define MAX_THREAD 64

static int thread_count_start, thread_count_now, pauseuid = -1;
static SceUID thread_buf_start[MAX_THREAD], thread_buf_now[MAX_THREAD], thid1 = -1, thid2 = -1;

static void pause_game(SceUID thid)
{
	if(pauseuid >= 0)
		return;
	pauseuid = thid;
	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, thread_buf_now, MAX_THREAD, &thread_count_now);
	int x, y, match;
	for(x = 0; x < thread_count_now; x++)
	{
		match = 0;
		SceUID tmp_thid = thread_buf_now[x];
		for(y = 0; y < thread_count_start; y++)
		{
			if((tmp_thid == thread_buf_start[y]) || (tmp_thid == thid1) || (tmp_thid == thid2))
			{
				match = 1;
				break;
			}
		}
		if(match == 0)
			sceKernelSuspendThread(tmp_thid);
	}
}

static void resume_game(SceUID thid)
{
	if(pauseuid != thid)
		return;
	pauseuid = -1;
	int x, y, match;
	for(x = 0; x < thread_count_now; x++)
	{
		match = 0;
		SceUID tmp_thid = thread_buf_now[x];
		for(y = 0; y < thread_count_start; y++)
		{
			if((tmp_thid == thread_buf_start[y]) || (tmp_thid == thid1) || (tmp_thid == thid2))
			{
				match = 1;
				break;
			}
		}
		if(match == 0)
			sceKernelResumeThread(tmp_thid);
	}
}

#define PATH_M_USBHOST "ms0:/SCEP/modules/usbhostfs.prx"
#define PATH_USBHOST "ms0:/SCEP/usbhost"

int main_thread(SceSize args, void *argp)
{	
	pause_game(thid1);
	int fd = sceIoOpen(PATH_USBHOST, PSP_O_RDONLY, 0777);
	if (fd > 0)
	{
		int mod;
		if(!sceKernelFindModuleByName("USBHostFS"))
		{
			mod = sceKernelLoadModule(PATH_M_USBHOST, 0, NULL);
			if(mod >= 0)
				mod = sceKernelStartModule(mod, strlen(PATH_M_USBHOST)+1, PATH_M_USBHOST, NULL, NULL);
			if(mod < 0)
			{
				sceIoClose(fd);
				resume_game(thid1);
				return 0;
			}
		}
		char iso_path[256];
		memset(iso_path, 0, 256);
		sceIoRead(fd, iso_path, 256);
		sceIoClose(fd);
		sceIoRemove(PATH_USBHOST);
		
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
		sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0); 
		sceUsbStart(HOSTFSDRIVER_NAME, 0, 0); 
		sceUsbActivate(HOSTFSDRIVER_PID);
		
		SEConfig config;
		sctrlSEGetConfig(&config);
		if (config.usenoumd)
		{
			sctrlSEMountUmdFromFile(iso_path, 1, 1);
		}
		else
		{
			sctrlSEMountUmdFromFile(iso_path, 0, config.useisofsonumdinserted);
		}
	}
	resume_game(thid1);
	
	return sceKernelExitDeleteThread(0);
}

/* Entry point */
int module_start(SceSize args, void *argp)
{
	thid1 = thid2 = -1;
	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, thread_buf_start, MAX_THREAD, &thread_count_start);
	thid1 = sceKernelCreateThread("SCX_MG", main_thread, 0x20, 0x800, 0, NULL);
	if (thid1 >= 0)
		sceKernelStartThread(thid1, args, argp);
	return 0;
}

/* Module stop entry */
int module_stop(SceSize args, void *argp)
{
	return 0;
}


