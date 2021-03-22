/*------------------------------------------------------
	Project:		PSP-CFW*
	Module:			CFW_Recovery
	Maintainer:		
------------------------------------------------------*/

#ifndef _CFW_UTIL_
#define _CFW_UTIL_

#include "type.h"

extern int getMaxlen(const char ** item, int item_num);

extern int readLine(SceUID fd, char *str);

extern void disableUsb(void);

extern int loadstartModule(char * path);

extern void enableUsb(int device);

#endif /*_CFW_UTIL_*/
