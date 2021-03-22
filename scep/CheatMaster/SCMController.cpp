/*	
	SCEP-CheatMaster 1.0
    Copyright (C) 2003-2007 

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

#include "SCMController.h"

int ScmController::MAX_THREAD = 64;
int	ScmController::thread_count_start = 0;
int	ScmController::thread_count_now = 0;
int	ScmController::thread_count_scm = 0;
SceUID ScmController::pauseuid = -1;
SceUID ScmController::heap_id = -1;
SceUID * ScmController::thread_buf_start = NULL;
SceUID * ScmController::thread_buf_now = NULL;
SceUID * ScmController::thread_buf_scm = NULL;

bool ScmController::init()
{
	heap_id = sceKernelCreateHeap(1, 1024 * 1, 1, "thread_heap");
	if(heap_id < 0)
	{
		return 0;
	}
	thread_buf_start = (SceUID *)sceKernelAllocHeapMemory(heap_id, MAX_THREAD * sizeof(SceUID));
	thread_buf_now = (SceUID *)sceKernelAllocHeapMemory(heap_id, MAX_THREAD * sizeof(SceUID));
	thread_buf_scm = (SceUID *)sceKernelAllocHeapMemory(heap_id, 10 * sizeof(SceUID));
	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, thread_buf_start, MAX_THREAD, &thread_count_start);
	return 1;
}

void ScmController::free()
{
	if(heap_id >= 0)
	{
		sceKernelFreeHeapMemory(heap_id, thread_buf_start);
		sceKernelFreeHeapMemory(heap_id, thread_buf_now);
		sceKernelFreeHeapMemory(heap_id, thread_buf_scm);
		sceKernelDeleteHeap(heap_id);
	}
}

int ScmController::addThread(SceUID thid)
{
	thread_buf_scm[thread_count_scm ++] = thid;
	return thread_count_scm;
}

void ScmController::pause(SceUID thid)
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
			if(tmp_thid == thread_buf_start[y])
			{
				match = 1;
				break;
			}
		}
		for(y = 0; y < thread_count_scm; y++)
		{
			if(tmp_thid == thread_buf_scm[y])
			{
				match = 1;
				break;
			}
		}
		if(match == 0)
			sceKernelSuspendThread(tmp_thid);
	}
}

void ScmController::resume(SceUID thid)
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
			if(tmp_thid == thread_buf_start[y])
			{
				match = 1;
				break;
			}
		}
		for(y = 0; y < thread_count_scm; y++)
		{
			if(tmp_thid == thread_buf_scm[y])
			{
				match = 1;
				break;
			}
		}
		if(match == 0)
			sceKernelResumeThread(tmp_thid);
	}
}

ScmController::ScmController()
{
	last_btn = 0;
	last_tick = 0;
	repeat_flag = 0;
}

u32 ScmController::read()
{
	sceCtrlPeekBufferPositive(&ctl, 1);
	if (ctl.Buttons == last_btn)
	{
		if (ctl.TimeStamp - last_tick < (repeat_flag ? CTRL_REPEAT_INTERVAL : CTRL_REPEAT_TIME)) return 0;
		repeat_flag = 1;
		last_tick = ctl.TimeStamp;
		return last_btn;
	}
	repeat_flag = 0;
	last_tick = ctl.TimeStamp;
	last_btn  = ctl.Buttons;
	return last_btn;
}

void ScmController::waitrelease()
{
	do {
		sceCtrlPeekBufferPositive(&ctl, 1);
		sceKernelDelayThread(20000);
	} while ((ctl.Buttons & 0xF0FFFF) != 0);
	last_tick = ctl.TimeStamp;
	last_btn  = ctl.Buttons;
}

u32 ScmController::waitmask(u32 keymask)
{
	u32 key;
	while((key = (read() & keymask)) == 0)
	{
		sceKernelDelayThread(20000);
	}
	return key;
}

u32 ScmController::waittime(u32 t)
{
	int i = 0, m = t * 50;
	u32 key;
	while((key = (read() & 0xF0FFFF)) == 0 && i < m)
	{
		sceKernelDelayThread(20000);
		++ i;
	}
	return key;
}

u32 ScmController::input()
{
	waitrelease();
	u32 key, key2;
	do {
		sceCtrlPeekBufferPositive(&ctl, 1);
		key = ctl.Buttons & 0xF1FFFF;
	} while(key == 0);
	key2 = key;
	while((key2 & key) == key)
	{
		sceKernelDelayThread(20000);
		key = key2;
		sceCtrlPeekBufferPositive(&ctl, 1);
		key2 = ctl.Buttons & 0xF1FFFF;
	}
	waitrelease();
	return key;
}

void ScmController::getKeyName(u32 key, char * res)
{
	res[0] = 0;
	if((key & PSP_CTRL_CIRCLE) > 0)
		strcat(res, "O");
	if((key & PSP_CTRL_LTRIGGER) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "L");
	}
	if((key & PSP_CTRL_RTRIGGER) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "R");
	}
	if((key & PSP_CTRL_CROSS) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "X");
	}
	if((key & PSP_CTRL_SQUARE) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "[]");
	}
	if((key & PSP_CTRL_TRIANGLE) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "^");
	}
	if((key & PSP_CTRL_UP) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "UP");
	}
	if((key & PSP_CTRL_DOWN) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "DOWN");
	}
	if((key & PSP_CTRL_LEFT) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "LEFT");
	}
	if((key & PSP_CTRL_RIGHT) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "RIGHT");
	}
	if((key & PSP_CTRL_HOME) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "HOME");
	}
	if((key & PSP_CTRL_SELECT) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "SELECT");
	}
	if((key & PSP_CTRL_START) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "START");
	}
	if((key & PSP_CTRL_VOLUP) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "VOLUP");
	}
	if((key & PSP_CTRL_VOLDOWN) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "VOLDOWN");
	}
	if((key & PSP_CTRL_NOTE) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "NOTE");
	}
	if((key & PSP_CTRL_SCREEN) > 0)
	{
		if(res[0] != 0)
			strcat(res, "+");
		strcat(res, "SCREEN");
	}
}
