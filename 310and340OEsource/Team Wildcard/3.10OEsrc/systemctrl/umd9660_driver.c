// dummy functions to allow the rest to compile

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman_kernel.h>
#include <psperror.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "malloc.h"

#include "isofs_driver.h"
char *GetUmdFile()
{
	return 0;
}

void SetUmdFile(char *file)
{
	return;
}

int  OpenIso()
{
	return 0;
}

int  ReadUmdFileRetry(void *buf, int size, int fpointer)
{
	return 0;
}

int  Umd9660ReadSectors(int lba, int nsectors, void *buf, int *eod)
{
	return 0;
}

int  Umd9660ReadSectors2(int lba, int nsectors, void *buf, int *eod)
{
	return 0;
}

PspIoDrv *getumd9660_driver()
{
	return 0;
}

void DoAnyUmd()
{
	return;
}
