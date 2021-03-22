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

#ifndef _SCMDEBUGTOOLS_H_
#define _SCMDEBUGTOOLS_H_

#include "SCMTypes.h"
#include "SCMString.h"
#include "SCMGraphic.h"

class SCMDebugTools
{
	public:
		SCMDebugTools(ScmGraphic * s);
		
		void memStatus();
		void modList();
		void thList();
		void findExports(const char * fnc_name);
		
	private:
		ScmGraphic * scr;
		u32 NameToNid(const char *name);
};

#endif /*_SCMDEBUGTOOLS_H_*/
