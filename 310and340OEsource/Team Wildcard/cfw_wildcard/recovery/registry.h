/*------------------------------------------------------
	Project:		PSP-CFW*
	Module:			CFW_Recovery
	Maintainer:		
------------------------------------------------------*/

#ifndef _CFW_REGISTRY_
#define _CFW_REGISTRY_

#include "type.h"

extern int getRegistryValue(const char *dir, const char *name, u32 *val);

extern int setRegistryValue(const char *dir, const char *name, u32 val);

#endif /*_CFW_REGISTRY_*/
