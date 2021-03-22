/*	
	SCEP-XMB 1.1
    Copyright (C) 1997-2004 

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   
*/

#ifndef _TEXT_H_
#define _TEXT_H_

static const char * yesno_item[] =
{
	"no",
	"yes"
};

static const char * main_item[] =
{
	"Capture",
	"sysCtrl",
	"FileMgr",
	"E-watch",
	"SCXconf",
	"PsPower"
};

static const char * power_item[] =
{
	"Sleep",
	"PowOFF",
	"Reboot",
	"Lock"
};

static const char * sysctrl_item[] =
{
	"LCDCtrl",
	"CPUSpeed",
	"OEConfig",
	"CronList",
	"SysInfo",
	"ThemeMgr"
};

static const char * oeconfig_item[] =
{
	"No-UMD",
	"150Kernel",
	"SkipLogo",
	"PlainMod",
	"Boot.bin",
	"ISOfs"
};

static const char * ewatch_item[] =
{
	"stopWatch",
	"Alarm"
};

#define BRIGHTNESS_MIN "0"
#define BRIGHTNESS_MAX "100"
#define SPEED_MIN "33"
#define SPEED_MAX "333"

#define USBHOST_MODULE "ms0:/SCEP/modules/usbhostfs.prx"
#define SCEPIO_MODULE "ms0:/SCEP/modules/scepIo.prx"
#define SCEPCAPTURE_MODULE "ms0:/SCEP/modules/scepCapture.prx"

#endif
