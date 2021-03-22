#include <pspsdk.h>
#include <pspkernel.h>
#include <pspaudio.h>
#include <pspaudiolib.h>
#include <psppower.h>
#include <pspctrl.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include <pspdisplay.h>

#define printf pspDebugScreenPrintf

void copyfile(char *, char *);
void writefile(unsigned char *, char *, int);
void deletefile(char *);
void renamefile(char *, char *);
unsigned int iserr(unsigned int);
char *getError(unsigned int);
void pause_for_cross();
void pause_for_triangle();
void verify_fail();
void reassign_fail();
void folder_fail();
