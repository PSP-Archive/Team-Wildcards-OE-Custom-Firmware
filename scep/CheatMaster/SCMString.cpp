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

#include "SCMString.h"

u8 ScmString::ZERO[] = {0, 0, 0, 0};

ScmString::ScmString(int len = 128, int cs = 1)
{
	bid = sceKernelCreateHeap(2, len * cs + 2, 1, "str_buffer");
	ScmString(bid, len, cs);
}

ScmString::ScmString(SceUID id, int len = 128, int cs = 1)
{
	heapid = id;
	max_len = len;
	char_size = cs;
	string = (char *)sceKernelAllocHeapMemory(heapid, char_size * len + 1);
}

void ScmString::destroy()
{
	if(heapid >= 0)
	{
		sceKernelFreeHeapMemory(heapid, string);
	}
	if(bid > 0)
	{
		sceKernelDeleteHeap(bid);
	}
}

ScmString::~ScmString()
{
	destroy();
}

int ScmString::read(const void * s)
{
	char * str = (char *)s;
	int n = 0;
	while(n < max_len * char_size && memcmp(str, ZERO, char_size))
	{
		memcpy(string + n, str + n, char_size);
		n += char_size;
	}
	memcpy(string + n, ZERO, char_size);
	str_len = n / char_size;
	return str_len;
}

int ScmString::read(const void * s, int len)
{
	char * str = (char *)s;
	int n = 0;
	while(n < max_len * char_size && n < len * char_size)
	{
		memcpy(string + n, str + n, char_size);
		n += char_size;
	}
	memcpy(string + n, ZERO, char_size);
	str_len = n / char_size;
	return str_len;
}

int ScmString::render(ScmGraphic & scr, bool flag)
{
	ScmGraphic::DrawCharError error;
	scr.pen.copy(scr.p0);
	int n = 0, nl = 0;
	u32 ch = 0;
	while(1)
	{
		switch(char_size)
		{
			case 1:
				ch = string[n];
				break;
			case 2:
				ch = *((u16 *)string + n);
				break;
			case 4:
				ch = *((u32 *)string + n);
				break;
		}
		if(!ch)
		{
			return n;
		}
		error = scr.drawChar(ch, flag);
		if(error == ScmGraphic::ENDLINE || error == ScmGraphic::NEWLINE)
		{
			nl ++;
			scr.pen.copy(scr.p0);
			scr.pen.addY(scr.line_height * nl);
			if(error == ScmGraphic::NEWLINE)
			{
				n ++;
			}
		}
		else if(error == ScmGraphic::ENDPAGE)
		{
			return n;
		}
		else
		{
			n ++;
		}
	}
	return -1;
}
