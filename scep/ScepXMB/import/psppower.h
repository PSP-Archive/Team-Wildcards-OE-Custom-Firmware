#ifndef _PSPPOWER_H_
#define _PSPPOWER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Simply reboots the PSP
 *
 */
int scePowerReboot();

/**
 * Request the PSP to go into standby
 *
 * @return 0 always
 */
int scePowerRequestStandby();

/**
 * Request the PSP to go into suspend
 *
 * @return 0 always
 */
int scePowerRequestSuspend();

#ifdef __cplusplus
}
#endif

#endif
