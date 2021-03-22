/*------------------------------------------------------
	Project:		PSP-CFW*
	Module:			CFW_Recovery
	Maintainer:		
------------------------------------------------------*/

#ifndef _PSPRTC_H_
#define _PSPRTC_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	u16 year;
	u16 month;
	u16 day;
	u16 hour;
	u16 minutes;
	u16 seconds;
	u32 microseconds;
} pspTime;

/**
 * Get current local time into a pspTime struct
 *
 * @param time - pointer to pspTime struct to receive time
 * @return 0 on success, < 0 on error
 */
int sceRtcGetCurrentClockLocalTime(pspTime *time);

#ifdef __cplusplus
}
#endif

#endif
