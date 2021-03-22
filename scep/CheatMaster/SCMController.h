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

#ifndef _SCMCONTROLLER_H_
#define _SCMCONTROLLER_H_

#include "SCMTypes.h"

class ScmController
{
	public:
		static int MAX_THREAD,
				thread_count_start,
				thread_count_now,
				thread_count_scm;
		static SceUID pauseuid, heap_id;
		static SceUID * thread_buf_start;
		static SceUID * thread_buf_now;
		static SceUID * thread_buf_scm;
		static bool init();
		static void free();
		static int addThread(SceUID thid);
		static void pause(SceUID thid);
		static void resume(SceUID thid);
		
		enum pspCtrlParam
		{
			CTRL_REPEAT_TIME = 0x40000,
			CTRL_REPEAT_INTERVAL = 0x12000
		};
		
		ScmController();
		
		u32 read();
		void waitrelease();
		u32 waitmask(u32 keymask);
		u32 waittime(u32 t);
		u32 input();
		void getKeyName(u32 key, char * res);
		
	private:
		SceCtrlData ctl;
		u32 last_btn;
		u32 last_tick;
		u32 repeat_flag;
};

#endif /*_SCMCONTROLLER_H_*/
