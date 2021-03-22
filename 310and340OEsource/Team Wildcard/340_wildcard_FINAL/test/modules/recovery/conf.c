/*------------------------------------------------------
	Project:		PSP-CFW*
	Module:			CFW_Config
	Maintainer:		
------------------------------------------------------*/

#include "conf.h"

extern bool cfwGetConfig(cfwConfig_p config)
{
	memset(config, 0, sizeof(cfwConfig));
	SceUID fd = sceIoOpen("flash1:/config.se", PSP_O_RDONLY, 0644);
	if (fd < 0)
	{
		return false;
	}
	if (sceIoRead(fd, config, sizeof(cfwConfig)) < sizeof(cfwConfig))
	{
		sceIoClose(fd);
		return false;
	}
	sceIoClose(fd);
	return true;
}

extern bool cfwSetConfig(cfwConfig_p config)
{
	sceIoRemove("flash1:/config.se");
	SceUID fd = sceIoOpen("flash1:/config.se", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fd < 0)
	{
		return false;
	}
	config->magic = CONFIG_MAGIC;
	if (sceIoWrite(fd, config, sizeof(cfwConfig)) < sizeof(cfwConfig))
	{
		sceIoClose(fd);
		return false;
	}
	sceIoClose(fd);
	return true;
}
