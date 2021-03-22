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
#include <psppower.h>
#include <pspmodulemgr_kernel.h>
#include <psputilsforkernel.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "util.h"

extern void setCpuspeed(int cpu)
{
	scePowerSetClockFrequency(222, 222, 111);
	scePowerSetCpuClockFrequency(222);
	scePowerSetBusClockFrequency(111);
	if (cpu >= 222)
	{
		scePowerSetClockFrequency(cpu, cpu, cpu/2);
	}
	else
	{
		scePowerSetCpuClockFrequency(cpu);
		scePowerSetBusClockFrequency(cpu/2);
	}
}

extern void getTimeString(char * times, int count)
{
	int tmp = count;
	int ms = 0, s = 0, m = 0, h = 0;
	ms = tmp % 60;
	tmp /= 60;
	s = tmp % 60;
	tmp /= 60;
	m = tmp % 60;
	h = tmp / 60;
	sprintf(times, "%02u:%02u:%02u'%02u\"", h, m, s, ms);
}

static u32 NameToNid(const char *name)
{
	u8 digest[20];
	u32 nid;
	if(sceKernelUtilsSha1Digest((u8 *)name, strlen(name), digest) >= 0)
	{
		nid = digest[0] | (digest[1] << 8) | (digest[2] << 16) | (digest[3] << 24);
		return nid;
	}
	return 0;
}

extern u32 * pspModuleExportHelper(const char * mod_name, const char * lib, const char * method_name, u32 * newProcAddr)
{
	SceModule * mod;
	mod = sceKernelFindModuleByName(mod_name);
	u32 nid = NameToNid(method_name);
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

#define USBHOST_MODULE "ms0:/SCEP/modules/usbhostfs.prx"
#define SCEPIO_MODULE "ms0:/SCEP/modules/scepIo.prx"
#define SCEPCAPTURE_MODULE "ms0:/SCEP/modules/scepCapture.prx"

extern int loadstartModule(const char * mod_name, char * path)
{
	int mod = -1;
	if(!sceKernelFindModuleByName(mod_name))
	{
		mod = sceKernelLoadModule(path, 0, NULL);
		if(mod >= 0)
			mod = sceKernelStartModule(mod, strlen(path)+1, path, NULL, NULL);
	}
	return mod;
}

extern int stopunloadModule(const char * name, int mod)
{
	int error = -1;
	if(sceKernelFindModuleByName(name))
	{
		int status;
		error = sceKernelStopModule(mod, 0, NULL, &status, NULL);
		if(error >= 0)
			error = sceKernelUnloadModule(mod);
		
	}
	return error;
}

