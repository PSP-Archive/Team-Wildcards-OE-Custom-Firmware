/*	
	SCEP-Series Memory library
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
#include <pspsysmem_kernel.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void * pspAlloc(SceUID * blockID, const char * name, SceSize size, int flag)
{
	int i, pid = -1;
	SceSize memsize;
	for (i = 3; i >= 2; i--)
	{
		memsize = sceKernelPartitionMaxFreeMemSize(i);
		if (memsize > size)
		{
			pid = i;
			if(flag)
				size = memsize;
			else break;
		}
	}
	if(pid < 0)
		return NULL;

	*blockID = sceKernelAllocPartitionMemory(pid, name, PSP_SMEM_Low, size, NULL);
	if(*blockID < 0)
		return NULL;
	return sceKernelGetBlockHeadAddr(*blockID);
}

extern int pspFree(SceUID blockid)
{
	if(blockid >= 0)
		return sceKernelFreePartitionMemory(blockid);
	return -1;
}
