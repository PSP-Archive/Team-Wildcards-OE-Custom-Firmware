/*------------------------------------------------------
	Project:		PSP-CFW*
	Module:			CFW_Recovery
	Maintainer:		
------------------------------------------------------*/

#include "type.h"
#include "ctrl.h"
#include "menu.h"
#include <pspdisplay.h>

PSP_MODULE_INFO("Recovery", 0x1000, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

#define MAX_THREAD 64

static int thread_count_start, thread_count_now;
static SceUID pauseuid = -1, thread_buf_start[MAX_THREAD], thread_buf_now[MAX_THREAD], thid1 = -1;

void pauseGame(SceUID thid)
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
			if((tmp_thid == thread_buf_start[y]) || (tmp_thid == thid1))
			{
				match = 1;
				break;
			}
		}
		if(match == 0)
			sceKernelSuspendThread(tmp_thid);
	}
}

void resumeGame(SceUID thid)
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
			if((tmp_thid == thread_buf_start[y]) || (tmp_thid == thid1))
			{
				match = 1;
				break;
			}
		}
		if(match == 0)
			sceKernelResumeThread(tmp_thid);
	}
}

int main_thread(SceSize args, void *argp)
{
	while(!sceKernelFindModuleByName("sceKernelLibrary"))
		sceKernelDelayThread(1000000);
	while(1)
	{
		if (sceKernelDevkitVersion() > 0x01050001)
		{
			while(1)
			{
				if(ctrlWaitMask(PSP_CTRL_RTRIGGER | PSP_CTRL_SELECT) == (PSP_CTRL_RTRIGGER | PSP_CTRL_SELECT))
				{
					break;
				}
			}
		}
		thid1 = sceKernelGetThreadId();
		pauseGame(thid1);
		void * vram;
		int bufferwidth = 512, pixelformat = PSP_DISPLAY_PIXEL_FORMAT_8888, unk = 1;
		vram = (void *) (0x40000000 | 0x04000000);
		if (sceKernelDevkitVersion() > 0x01050001)
		{
			sceDisplayGetFrameBuf(&vram, &bufferwidth, &pixelformat, &unk);
			vram = (void *) ((u32)vram | 0x40000000);
		}
		sceDisplaySetMode(0, SCREEN_WIDTH, SCREEN_HEIGHT);
		sceDisplaySetFrameBuf((void *) vram, bufferwidth, pixelformat, unk);
		sceKernelSetDdrMemoryProtection((void *)0x88400000, 0x00400000, 0xF);
		SceUID heapid = sceKernelCreateHeap(5, 1024 * 1024, 1, "graphic_heap");
		if(heapid >= 0 && initGraphic(heapid, vram))
		{
			mainMenu();
			freeGraphic();
			sceKernelDeleteHeap(heapid);
		}
		resumeGame(thid1);
		//sceKernelExitVSHVSH(NULL);
	}
	return 0;
}

int module_start(SceSize args, void *argp)
{
	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, thread_buf_start, MAX_THREAD, &thread_count_start);
	SceUID thid1 = sceKernelCreateThread("recovery_thread", main_thread, 47, 0x4000, 0x00100001, NULL);
	if(thid1 >= 0)
	{
		sceKernelStartThread(thid1, 0, 0);
	}
	return 0;
}

int module_stop(SceSize args, void *argp)
{
	return 0;
}
