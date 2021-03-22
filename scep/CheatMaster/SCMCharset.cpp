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

#include "ScmCharset.h"

void ScmCharset::fromUTF8(ScmString & des, const ScmString & src)
{
	u8 * srcstr = (u8 *)src.string;
	u16 * desstr = (u16 *)des.string;
	while(*srcstr)
	{
		if(*srcstr < 0x80)
		{
			*(desstr++) = *(srcstr++);
		}
		else if((*srcstr >> 5) == 6 && (*(srcstr + 1) >> 6) == 2)
		{
			*(desstr++) = (*srcstr & 0x1F) << 6 | (*(srcstr + 1) & 0x3F);
			srcstr += 2;
		}
		else if((*srcstr >> 4) == 14 && (*(srcstr + 1) >> 6) == 2 && (*(srcstr + 2) >> 6) == 2)
		{
			*(desstr++) = (*(srcstr) & 0x0F) << 12 | (*(srcstr + 1) & 0x3F) << 6 | (*(srcstr + 2) & 0x3F);
			srcstr += 3;
		}
	}
	*desstr = 0;
}
