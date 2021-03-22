// modplayer.h: headers for psp modplayer code
//
// All public functions for modplayer
//
//////////////////////////////////////////////////////////////////////
#ifndef _OGGPLAYER_H_
#define _OGGPLAYER_H_

#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>
#include "codec.h"

#ifdef __cplusplus
extern "C" {
#endif

//  Function prototypes for public functions
    void OGGsetStubs(codecStubs * stubs);

// private functions
    void OGG_Init(int channel);
    int OGG_Play();
    void OGG_Pause();
    int OGG_Stop();
    void OGG_End();
    int OGG_Load(char *filename);
    void OGG_Tick();
    void OGG_Close();
    void OGG_FreeTune();
    int OGG_IsSeekable();
    void OGG_Restart();
    void OGG_Rewind();
    void OGG_FastForward();
    void OGG_GetInfo(char *buf);
    void OGG_GetTimeString(char *dest);
    int OGG_EndOfStream();


#ifdef __cplusplus
}
#endif
#endif
