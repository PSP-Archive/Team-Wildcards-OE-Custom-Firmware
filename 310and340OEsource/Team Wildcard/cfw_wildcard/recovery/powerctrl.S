.set noreorder

#include "pspimport.s"

IMPORT_START	"scePower_driver",0x00010000
IMPORT_FUNC		"scePower_driver",0x0442D852,scePowerReboot
IMPORT_FUNC		"scePower_driver",0x2B7C7CF4,scePowerRequestStandby
IMPORT_FUNC		"scePower_driver",0xAC32C9CC,scePowerRequestSuspend
