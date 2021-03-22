#include <pspsdk.h>
#include <pspkernel.h>
#include <pspmodulemgr_kernel.h>
#include <pspthreadman_kernel.h>
#include <pspinit.h>
#include <kubridge.h>

#include <string.h>

//0x00005D14
SceUID kuKernelLoadModule(const char *path, int flags, SceKernelLMOption *option)
{
	int k1, res;

	k1 = pspSdkSetK1(0);
	res = sceKernelLoadModule(path, flags, option);
	pspSdkSetK1(k1);

	return res;
}

//0x00005D80
SceUID kuKernelLoadModuleWithApitype2(int apitype, const char *path, int flags, SceKernelLMOption *option)
{
	int k1, res;

	k1 = pspSdkSetK1(0);
	res = sceKernelLoadModuleWithApitype2(apitype, path, flags, option);
	pspSdkSetK1(k1);

	return res;
}

//0x00005DFC
int kuKernelInitApitype()
{
	return sceKernelInitApitype();
}

//0x00005E04
int kuKernelInitFileName(char *filename)
{
	int k1 = pspSdkSetK1(0);

	strcpy(filename, sceKernelInitFileName());

	pspSdkSetK1(k1);
	return 0;
}

//0x00005E54
int kuKernelBootFrom()
{
	return sceKernelBootFrom();
}

//0x00005E5C
int kuKernelInitKeyConfig()
{
	return sceKernelInitKeyConfig();
}

//0x00005E64
int kuKernelGetUserLevel(void)
{
	int k1, res;

	k1 = pspSdkSetK1(0);
	res = sceKernelGetUserLevel();
	pspSdkSetK1(k1);

	return res;
}

//0x00005EA8
int kuKernelSetDdrMemoryProtection(void *addr, int size, int prot)
{
	int k1, res;

	k1 = pspSdkSetK1(0);
	res = sceKernelSetDdrMemoryProtection(addr, size, prot);
	pspSdkSetK1(k1);

	return res;
}
