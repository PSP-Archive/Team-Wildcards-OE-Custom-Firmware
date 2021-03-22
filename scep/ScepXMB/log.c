
#include <pspkernel.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int openlog(const char* szFile)
{
    return sceIoOpen(szFile, PSP_O_CREAT | PSP_O_APPEND | PSP_O_RDWR, 0777);
}
extern void printlog(int fd, const char* sz)
{
    sceIoWrite(fd, sz, strlen(sz));
}
extern void closelog(int fd)
{
    sceIoClose(fd);
}