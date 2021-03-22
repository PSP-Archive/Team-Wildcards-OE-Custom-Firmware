/*------------------------------------------------------
	Project:		PSP-CFW*
	Module:			CFW_autoboot_patch
	Maintainer:		
------------------------------------------------------*/

#include "autoboot.h"
#include <pspiofilemgr.h>

void patchEnableStartupProg()
{
	SceUID fd = -1;
	if (sceKernelDevkitVersion() > 0x01050001)
	{
        fd = sceIoOpen("flash0:/_real_path_kd/rtc.prx", PSP_O_RDWR, 0777);
	}
	else
	{
        fd = sceIoOpen("flash0:/kd/rtc.prx", PSP_O_RDWR, 0777);

	}
	if(fd >= 0)
	{
		u32 branch = 0x10000003;
		sceIoLseek32(fd, 0x250, SEEK_SET);
		sceIoWrite(fd, &branch, 4);
		sceIoClose(fd);
	}
}

void patchDisableStartupProg()
{
	SceUID fd = -1;
	if (sceKernelDevkitVersion() > 0x01050001)
	{
        fd = sceIoOpen("flash0:/_real_path_kd/rtc.prx", PSP_O_RDWR, 0777);
	}
	else
	{
        fd = sceIoOpen("flash0:/kd/rtc.prx", PSP_O_RDWR, 0777);

	}
	if(fd >= 0)
	{
		u32 nop = 0;
		sceIoLseek32(fd, 0x250, SEEK_SET);
		sceIoWrite(fd, &nop, 4);
		sceIoClose(fd);
	}
}
