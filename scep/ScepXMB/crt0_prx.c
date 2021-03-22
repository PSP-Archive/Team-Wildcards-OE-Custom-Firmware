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
#include <pspdisplay.h>
#include <psploadexec_kernel.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ctrl.h"
#include "ui.h"
#include "config.h"
#include <systemctrl_se.h>
#include <systemctrl.h>
#include <pspmodulemgr_kernel.h>

PSP_MODULE_INFO("SCEP_XMB", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);
PSP_MAIN_THREAD_STACK_SIZE_KB(0);

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

/*int ss_main_thread(SceSize args, void *argp) {
	thid2 = sceKernelGetThreadId();
	screenshot_init();
	while(1)
	{
		SceCtrlData ctl;
		do {
			sceKernelDelayThread(20000);
			sceCtrlPeekBufferPositive(&ctl, 1);
		} while((ctl.Buttons & 0xF0FFFF) != PSP_CTRL_VOLDOWN);

		Screenshot();
	}
	return 0;
}

void start_ssthread()
{
	thid2 = sceKernelCreateThread("Screenshot main_thread", ss_main_thread, 47, 0x1000, 0, NULL);
	if(thid2 >= 0)
		sceKernelStartThread(thid2, 0, 0);
}

void stop_ssthread()
{
	sceKernelTerminateThread(thid2);
	thid2 = -1;
}*/

int main_thread(SceSize args, void *argp)
{
	while(!sceKernelFindModuleByName("sceKernelLibrary"))
		sceKernelDelayThread(1000000);
	
	scxOption * scx = loadConfig();
	
	sceIoUnassign("flash0:");
	sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:",  IOASSIGN_RDWR, NULL, 0);
	
	/*sceKernelDelayThread(10000000);
	
	SceUID mods[100];
	int mod_count, i;
	SceModule *pMod;
	sceKernelGetModuleIdList(mods, 100 * sizeof(SceUID), &mod_count);
	int log = openlog("ms0:/modlist.txt");
	char s[128];
	for(i = 0; i < mod_count; i++)
	{
		pMod = sceKernelFindModuleByUID(mods[i]);
		if(pMod)
		{
			printlog(log, pMod->modname);
			sprintf(s, " 0x%08X\n", mods[i]);
			printlog(log, s);
		}
	}
	closelog(log);
	SceKernelThreadInfo thinfo;
	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, mods, 100, &mod_count);
	log = openlog("ms0:/thlist.txt");
	for(i = 0; i < mod_count; i++)
	{
		memset(&thinfo, 0, sizeof(thinfo));
		thinfo.size = sizeof(thinfo);
		if(!sceKernelReferThreadStatus(mods[i], &thinfo))
		{
			printlog(log, thinfo.name);
			sprintf(s, "attr:0x%08X stat:0x%08X 0x%08X\n",thinfo.attr, thinfo.status, mods[i]);
			printlog(log, s);
		}
	}
	closelog(log);*/
	
	while(1)
	{
		ctrl_waitmask(scx->hotkey);
		thid1 = sceKernelGetThreadId();
		pause_game(thid1);
		sceDisplayWaitVblankStart();
		//start_ssthread();
		uiMain();
		//stop_ssthread();
		sceDisplayWaitVblankStart();
		resume_game(thid1);
		if(scx->iso)
		{
			scx->iso = 0;
			saveConfig();
			SEConfig config;
			sctrlSEGetConfig(&config);
			if (config.usenoumd)
				sctrlSEMountUmdFromFile(scx->iso_path, 1, 1);
			else
				sctrlSEMountUmdFromFile(scx->iso_path, 0, config.useisofsonumdinserted);
			char file[256];
			strcpy(file, "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN");
			int apitype = PSP_LOADMODULE_APITYPE_DISC;
			struct SceKernelLoadExecVSHParam param;
			memset(&param, 0, sizeof(param));
			if (!param.key)
				param.key = "game";
			param.size = sizeof(param);
			param.args = strlen(file)+1;
			param.argp = file;
			sctrlKernelLoadExecVSHWithApitype(apitype, file, &param);
		}
	}
	return sceKernelExitDeleteThread(0);
}

int module_start(SceSize args, void *argp)
{
	thid1 = thid2 = -1;
	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, thread_buf_start, MAX_THREAD, &thread_count_start);
	thid1 = sceKernelCreateThread("SCX_thread", main_thread, 47, 0x2000, 0x00100001, NULL);
	if(thid1 >= 0)
		sceKernelStartThread(thid1, 0, 0);
	return 0;
}

int module_stop(SceSize args, void *argp)
{
	return 0;
}

int module_reboot_before(SceSize args, void *argp)
{
	return 0;
}
