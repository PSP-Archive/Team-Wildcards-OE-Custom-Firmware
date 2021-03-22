.set noreorder

#include "pspimport.s"

IMPORT_START	"sceRtc_driver",0x00010000
IMPORT_FUNC		"sceRtc_driver",0xE7C27D1B,sceRtcGetCurrentClockLocalTime
