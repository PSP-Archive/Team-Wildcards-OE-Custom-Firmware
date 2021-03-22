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
#include "SCMTypes.h"
#include "SCMGraphic.h"
#include "SCMController.h"
#include "SCMDebugTools.h"
#include "SCMString.h"
#include "SCMCharset.h"

PSP_MODULE_INFO("SCEP_CheatMaster", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

int main_thread(SceSize args, void *argp)
{
	while(!sceKernelFindModuleByName("sceKernelLibrary"))
		sceKernelDelayThread(1000000);
	sceKernelDelayThread(5000000);
	
	SceUID heap_id = sceKernelCreateHeap(2, 1024 * 200, 1, "scm_heap");
	if(heap_id < 0)
	{
		return 0;
	}
	ScmGraphic &screen = *(ScmGraphic *)sceKernelAllocHeapMemory(heap_id, sizeof(ScmGraphic));
	ScmController &ctl = *(ScmController *)sceKernelAllocHeapMemory(heap_id, sizeof(ScmController));
	
	while(1)
	{
		ctl.waitmask(PSP_CTRL_NOTE);
		SceUID thid1 = sceKernelGetThreadId();
		ctl.pause(thid1);
		
		sceKernelSetDdrMemoryProtection((void *)0x88400000, 0x00400000, 0xF);
		SceUID bid = sceKernelCreateHeap(5, 1024 * 1024 * 3, 1, "graphic_heap");
		if(bid < 0)
		{
			return 0;
		}
		
		if(screen.init(bid))
		{
			//SCMDebugTools debug(&screen);
			//debug.memStatus();
			ScmString s(bid, 512, 1);
			ScmString str(bid, 256, 2);
			int fd = sceIoOpen("ms0:/test.txt", PSP_O_RDONLY, 0777);
			int byte = sceIoRead(fd, s.string, 512);
			sceIoClose(fd);
			s.string[byte] = 0;
			ScmCharset iconv;
			iconv.fromUTF8(str, s);
			
			screen.p0.setXY(100, 30);
			screen.p1.setXY(300, 230);
			screen.blur();
			int done = 0;
			while(!done)
			{
				u32 key = ctl.waitmask(PSP_CTRL_LEFT | PSP_CTRL_RIGHT | PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_CROSS);
				screen.clearRect(0);
				switch(key)
				{
					case PSP_CTRL_LEFT:
						screen.p0.addX(-10);
						screen.p1.addX(-10);
						if(screen.p0.x < 10)
						{
							screen.p0.setX(10);
							screen.p1.setX(210);
						}
						break;
					case PSP_CTRL_RIGHT:
						screen.p0.addX(10);
						screen.p1.addX(10);
						if(screen.p1.x > 469)
						{
							screen.p0.setX(277);
							screen.p1.setX(477);
						}
						break;
					case PSP_CTRL_UP:
						screen.p0.addY(-10);
						screen.p1.addY(-10);
						if(screen.p0.y < 10)
						{
							screen.p0.setY(10);
							screen.p1.setY(210);
						}
						break;
					case PSP_CTRL_DOWN:
						screen.p0.addY(10);
						screen.p1.addY(10);
						if(screen.p1.y > 261)
						{
							screen.p0.setY(69);
							screen.p1.setY(269);
						}
						break;
					case PSP_CTRL_CROSS:
						done = 1;
						break;
				}
				screen.blur();
				screen.p0.addY(20);
				str.render(screen, true);
				screen.p0.addY(-20);
			}
		}
		screen.free();
		
		if(bid >= 0)
		{
			sceKernelDeleteHeap(bid);
		}
		ctl.resume(thid1);
	}
	
	sceKernelFreeHeapMemory(heap_id, &screen);
	sceKernelFreeHeapMemory(heap_id, &ctl);
	sceKernelDeleteHeap(heap_id);
	
	return 0;
}

// fix exports

extern "C" {

int module_start(SceSize args, void *argp)
{
	if(!ScmController::init())
	{
		return -1;
	}
	SceUID thid1 = sceKernelCreateThread("SCM_thread", main_thread, 47, 0x3000, 0x00100001, NULL);
	if(thid1 >= 0)
	{
		ScmController::addThread(thid1);
		sceKernelStartThread(thid1, 0, 0);
	}
	return 0;
}

int module_stop(SceSize args, void *argp)
{
	ScmController::free();
	return 0;
}

int module_reboot_before(SceSize args, void *argp)
{
	ScmController::free();
	return 0;
}

} /* fix exports */

