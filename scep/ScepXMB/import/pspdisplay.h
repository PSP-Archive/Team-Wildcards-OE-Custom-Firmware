#ifndef _PSPDISPLAY_H_
#define _PSPDISPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set Display brightness to a particular level
 *
 * @param level - Level of the brightness. it goes from 0 (black screen) to 100 (max brightness)
 * @param unk1 - Unknown can be 0 or 1 (pass 0)
 */
void sceDisplaySetBrightness(int level,int unk1);

/**
 * Get current display brightness
 *
 * @param level - Pointer to int to receive the current brightness level (0-100)
 * @param unk1 - Pointer to int, receives unknown, it's 1 or 0
 */
void sceDisplayGetBrightness(int *level,int *unk1);

#ifdef __cplusplus
}
#endif

#endif
