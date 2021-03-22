/*------------------------------------------------------
	Project:		PSP-CFW*
	Module:			CFW_Recovery
	Maintainer:		
------------------------------------------------------*/

#include "util.h"
#include <pspusb.h>
#include <pspusbstor.h>

static int usbStatus = 0;
static int usbModuleStatus = 0;

static PspIoDrv *lflash_driver;
static PspIoDrv *msstor_driver;

extern int getMaxlen(const char ** item, int item_num)
{
	int i, maxlen = 0;
	for(i = 0; i < item_num; i ++)
	{
		if(strlen(item[i]) > maxlen)
		{
			maxlen = strlen(item[i]);
		}
	}
	return maxlen;
}

extern int readLine(SceUID fd, char *str)
{
	char ch = 0;
	int n = 0;

	while (1)
	{	
		if (sceIoRead(fd, &ch, 1) != 1)
			return n;

		if (ch < 0x20)
		{
			if (n != 0)
				return n;
		}
		else
		{
			*str++ = ch;
			n++;
		}
	}
	
}

int (* Orig_IoOpen)(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode);
int (* Orig_IoClose)(PspIoDrvFileArg *arg);
int (* Orig_IoRead)(PspIoDrvFileArg *arg, char *data, int len);
int (* Orig_IoWrite)(PspIoDrvFileArg *arg, const char *data, int len);
SceOff(* Orig_IoLseek)(PspIoDrvFileArg *arg, SceOff ofs, int whence);
int (* Orig_IoIoctl)(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
int (* Orig_IoDevctl)(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);

int unit;

/* 1.50 specific function */
PspIoDrv *FindDriver(char *drvname)
{
	u32 *mod = (u32 *)sceKernelFindModuleByName("sceIOFileManager");

	if (!mod)
	{
		return NULL;
	}

	u32 text_addr = *(mod+27);

	u32 *(* GetDevice)(char *) = (void *)(text_addr+0x16D4);
	u32 *u;

	u = GetDevice(drvname);

	if (!u)
	{
		return NULL;
	}

	return (PspIoDrv *)u[1];
}

static int New_IoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode)
{
	if (!lflash_driver->funcs->IoOpen)
		return -1;

	if (unit == 0)
		file = "0,0";
	else
		file = "0,1";

	return lflash_driver->funcs->IoOpen(arg, file, flags, mode);
}

static int New_IoClose(PspIoDrvFileArg *arg)
{
	if (!lflash_driver->funcs->IoClose)
		return -1;

	return lflash_driver->funcs->IoClose(arg);
}

static int New_IoRead(PspIoDrvFileArg *arg, char *data, int len)
{
	if (!lflash_driver->funcs->IoRead)
		return -1;

	return lflash_driver->funcs->IoRead(arg, data, len);
}
static int New_IoWrite(PspIoDrvFileArg *arg, const char *data, int len)
{
	if (!lflash_driver->funcs->IoWrite)
		return -1;

	return lflash_driver->funcs->IoWrite(arg, data, len);
}

static SceOff New_IoLseek(PspIoDrvFileArg *arg, SceOff ofs, int whence)
{
	if (!lflash_driver->funcs->IoLseek)
		return -1;

	return lflash_driver->funcs->IoLseek(arg, ofs, whence);
}

u8 data_5803[96] = 
{
	0x02, 0x00, 0x08, 0x00, 0x08, 0x00, 0x07, 0x9F, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x21, 0x21, 0x00, 0x00, 0x20, 0x01, 0x08, 0x00, 0x02, 0x00, 0x02, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static int New_IoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	if (cmd == 0x02125008)
	{
		u32 *x = (u32 *)outdata;
		*x = 1; /* Enable writing */
		return 0;
	}
	else if (cmd == 0x02125803)
	{
		memcpy(outdata, data_5803, 96);
		return 0;
	}

	return -1;
}

static int New_IoDevctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	if (cmd == 0x02125801)
	{
		u8 *data8 = (u8 *)outdata;

		data8[0] = 1;
		data8[1] = 0;
		data8[2] = 0,
		data8[3] = 1;
		data8[4] = 0;
				
		return 0;
	}

	return -1;
}

extern void disableUsb(void) 
{ 
	if(usbStatus) 
	{
		sceUsbDeactivate(0);
		sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0);
		sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);

		msstor_driver->funcs->IoOpen = Orig_IoOpen;
		msstor_driver->funcs->IoClose = Orig_IoClose;
		msstor_driver->funcs->IoRead = Orig_IoRead;
		msstor_driver->funcs->IoWrite = Orig_IoWrite;
		msstor_driver->funcs->IoLseek = Orig_IoLseek;
		msstor_driver->funcs->IoIoctl = Orig_IoIoctl;
		msstor_driver->funcs->IoDevctl = Orig_IoDevctl;

		usbStatus = 0;
	}
}

extern int loadstartModule(char * path)
{
	SceUID mod = sceKernelLoadModule(path, 0, NULL);
	if(mod >= 0)
		mod = sceKernelStartModule(mod, strlen(path)+1, path, NULL, NULL);
	return mod;
}

extern void enableUsb(int device) 
{
	if (usbStatus)
	{
		disableUsb();
		sceKernelDelayThread(300000);
	}

	if(!usbModuleStatus) 
	{
		loadstartModule("flash0:/kd/semawm.prx");
		loadstartModule("flash0:/kd/usbstor.prx");
		loadstartModule("flash0:/kd/usbstormgr.prx");
		loadstartModule("flash0:/kd/usbstorms.prx");
		loadstartModule("flash0:/kd/usbstorboot.prx");
		
		lflash_driver = FindDriver("lflash");
		msstor_driver = FindDriver("msstor");
		
		Orig_IoOpen = msstor_driver->funcs->IoOpen;
		Orig_IoClose = msstor_driver->funcs->IoClose;
		Orig_IoRead = msstor_driver->funcs->IoRead;
		Orig_IoWrite = msstor_driver->funcs->IoWrite;
		Orig_IoLseek = msstor_driver->funcs->IoLseek;
		Orig_IoIoctl = msstor_driver->funcs->IoIoctl;
		Orig_IoDevctl = msstor_driver->funcs->IoDevctl;

		usbModuleStatus = 1;
	}

	if (device != 0)
	{
		unit = device-1;

		msstor_driver->funcs->IoOpen = New_IoOpen;
		msstor_driver->funcs->IoClose = New_IoClose;
		msstor_driver->funcs->IoRead = New_IoRead;
		msstor_driver->funcs->IoWrite = New_IoWrite;
		msstor_driver->funcs->IoLseek = New_IoLseek;
		msstor_driver->funcs->IoIoctl = New_IoIoctl;
		msstor_driver->funcs->IoDevctl = New_IoDevctl;
	}

	sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
	sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
	sceUsbstorBootSetCapacity(0x800000);
	sceUsbActivate(0x1c8);
	usbStatus = 1;
}
