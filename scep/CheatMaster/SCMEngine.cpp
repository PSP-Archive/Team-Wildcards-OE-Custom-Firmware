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

#include <string>

#include "SCMEngine.h"

using namespace std;

ScmScript::ScmScript(const string &path)
{
	int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
	if(fd < 0)
	{
		ok = false;
		return;
	}
	SceIoStat stat;
	sceIoGetstat(path, &stat);
	char * buf = new char[stat.st_size];
	sceIoRead(fd, script, stat.st_size);
	script = buf;
	delete [] buf;
	int i = 0;
	while(i < script.length())
	{
		int j;
		if(script.at(i) == '/' && script.at(i + 1) == '*')
		{
			j = script.find("*/", i);
			if(j == string::npos)
			{
				ok = false;
				return;
			}
			script.erase(i, j - i + 2);
		}
		j = 0;
		while(script.at(i + j) == ' ' || script.at(i + j) == '\t' || script.at(i + j) == '\n' || script.at(i + j) == '\r' || script.at(i + j) == '\0' || script.at(i + j) == '\x0B')
		{
			j ++;
		}
		if(j)
		{
			script.erase(i, j);
		}
		
		i ++;
	}
}

int ScmScript::execute()
{
	
}
