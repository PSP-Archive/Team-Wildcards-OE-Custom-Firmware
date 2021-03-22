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

#include "SCMDebugTools.h"

SCMDebugTools::SCMDebugTools(ScmGraphic * s)
{
	scr = s;
}

void SCMDebugTools::memStatus()
{
	scr->p0.setXY(5, 10);
	scr->p1.setXY(464, 256);
	scr->drawRect(20, false, 0x0A05FFFF);
	scr->p0.setXY(50, 30);
	scr->drawString(L"當前內存狀況:(序號/大小/起始/總可用/塊可用/屬性)", true);
	scr->p0.setXY(50, 50);
	scr->drawString(L"繁體简体混合在一起看效果到底怎樣。！啦啦笔记交五个PSP的新資質资质", true);
	scr->p0.setXY(20, 60);
	scr->p1.setXY(460, 60);
	scr->drawLine();
	scr->p1.setXY(474, 266);
	PspSysmemPartitionInfo pinfo;
	char m[128];
	int i;
	for(i = 0; i < 6; i++)
	{
		memset(&pinfo, 0, sizeof(pinfo));
		pinfo.size = sizeof(pinfo);
		sceKernelQueryMemoryPartitionInfo(i + 1, &pinfo);
		sprintf(m, "%u  0x%08x  0x%08x  0x%08x 0x%08x 0x%x", i + 1, pinfo.memsize, pinfo.startaddr, sceKernelPartitionTotalFreeMemSize (i + 1), sceKernelPartitionMaxFreeMemSize (i + 1), pinfo.attr);
		scr->p0.addY(30);
		scr->drawString(m, true);
	}
}

void SCMDebugTools::modList()
{
	SceUID modid[100];
	int modcount;
	sceKernelGetModuleIdList(modid, 100, &modcount);
	scr->p0.setXY(0, 0);
	scr->p1.setXY(474, 266);
	scr->drawRect(20, false, 0x0505FFFF);
	scr->p0.setXY(20, 20);
	scr->drawString(L"已加載模塊列錶:", true);
	scr->p0.setXY(20, 30);
	scr->p1.setXY(460, 30);
	scr->drawLine();
	char s[64];
	sprintf(s, "(%d)", modcount);
	scr->p0.setXY(200, 20);
	scr->p1.setXY(474, 266);
	scr->drawString(s, true);
	SceKernelModuleInfo minfo;
	for(int i = 0; i < modcount; i++)
	{
		memset(&minfo, 0, sizeof(SceKernelModuleInfo));
		minfo.size = sizeof(SceKernelModuleInfo);
		sceKernelQueryModuleInfo(modid[i], &minfo);
		if(i < 15)
		{
			scr->p0.setXY(40, 50 + i * 15);
			scr->drawString(minfo.name, true);
		}
		else if(i < 30)
		{
			scr->p0.setXY(140, 50 + (i - 15) * 15);
			scr->drawString(minfo.name, true);
		}
		else if(i < 45)
		{
			scr->p0.setXY(240, 50 + (i - 30) * 15);
			scr->drawString(minfo.name, true);
		}
		else
		{
			scr->p0.setXY(340, 50 + (i - 45) * 15);
			scr->drawString(minfo.name, true);
		}
	}
}

void SCMDebugTools::thList()
{
	SceUID thid[100];
	int thcount;
	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, thid, 100, &thcount);
	scr->p0.setXY(0, 0);
	scr->p1.setXY(474, 266);
	scr->drawRect(20, false, 0x0505FFFF);
	scr->p0.setXY(20, 20);
	scr->drawString(L"線程列錶:", true);
	scr->p0.setXY(20, 30);
	scr->p1.setXY(460, 30);
	scr->drawLine();
	char s[64];
	sprintf(s, "(%d)", thcount);
	scr->p0.setXY(100, 20);
	scr->p1.setXY(474, 266);
	scr->drawString(s, true);
	SceKernelThreadInfo thinfo;
	for(int i = 0; i < thcount; i++)
	{
		memset(&thinfo, 0, sizeof(SceKernelThreadInfo));
		thinfo.size = sizeof(SceKernelThreadInfo);
		sceKernelReferThreadStatus(thid[i], &thinfo);
		if(i < 14)
		{
			scr->p0.setXY(20, 50 + i * 16);
			scr->drawString(thinfo.name, true);
		}
		else if(i < 28)
		{
			scr->p0.setXY(240, 50 + (i - 14) * 16);
			scr->drawString(thinfo.name, true);
		}
	}
}

u32 SCMDebugTools::NameToNid(const char *name)
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

void SCMDebugTools::findExports(const char * fnc_name)
{
	SceUID modid[100];
	int modcount;
	sceKernelGetModuleIdList(modid, 100, &modcount);
	scr->p0.setXY(5, 10);
	scr->p1.setXY(464, 256);
	scr->drawRect(20, false, 0x0A05FFFF);
	u32 nid = NameToNid(fnc_name);
	scr->p0.setY(40);
	for(int i = 0; i < modcount; i++)
	{
		scr->p0.setX(10);
		SceModule * mod = sceKernelFindModuleByUID(modid[i]);
		u32 * ent_next = (u32 *) mod->ent_top;
		u32 * ent_end = (u32 *) mod->ent_top + (mod->ent_size >> 2);
		while (ent_next < ent_end)
		{
			SceLibraryEntryTable* ent = (SceLibraryEntryTable*)ent_next;

			if (ent->libname[0])
	 		{
				int count = ent->stubcount + ent->vstubcount;
				u32* nidtable = (u32*)ent->entrytable;
				int i;
				for (i = 0; i < count; i++)
				{
					if (nidtable[i] == nid)
	 				{
						scr->drawString(L"找到函數:", true);
						scr->p0.setX(scr->pen.x + 10);
						scr->drawString(fnc_name, true);
						scr->p0.addY(20);
						scr->drawString(ent->libname, true);
						scr->p0.addY(20);
						scr->drawString(mod->modname, true);
						scr->p0.addY(20);
					}
				}
			} 
			ent_next +=  ent->len;  // len in 32-bit words. 
		}
	}
}
