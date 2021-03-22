/*------------------------------------------------------
	Project:		PSP-CFW*
	Module:			CFW_Recovery
	Maintainer:		
------------------------------------------------------*/

#ifndef _CFW_RECOVERY_TEXT_
#define _CFW_RECOVERY_TEXT_

static const char * title = "CFW 3.51 wildc*rd";

static const char * plugin_path[] =
{
	"ms0:/seplugins/vsh.txt",
	"ms0:/seplugins/game.txt",
	"ms0:/seplugins/pops.txt"
};

static const char * isEnter[] =
{
	"O is Enter now!",
	"X is Enter now!"
};

static const char * wma[] =
{
	"Activating WMA...",
	"WMA was already activated."
};

static const char * flashplayer[] =
{
	"Activating Flash Player...",
	"Flash Player was already activated."
};

static const char * enable_disable[] =
{
	"[disable]",
	"[enable]"
};

static const char * kernel[] =
{
	"[kernel3xx]",
	"[kernel150]"
};

static const char * fake_region[] =
{
	"[disable]",
	"[Japan]",
	"[America]",
	"[Europe]",
	"[Australia]",
	"[Russia]"
};

static const u8 fake_region_s = 6;

static const char * menu_main150[] =
{
	"Settings",
	"Extends",
	"Recovery",
	"Exit"
};

static const u8 menu_main150_s = 4;

static const char * menu_main3xx[] =
{
	"Settings",
	"Extends",
	"Power"
};

static const u8 menu_main3xx_s = 3;

static const char * menu_power[] =
{
	"sleep",
	"power off",
	"reboot"
};

static const u8 menu_power_s = 3;

static const char * menu_settings[] =
{
	"Base config",
	"Advanced",
	"Registry hack"
};

static const u8 menu_settings_s = 3;

static const char * menu_baseconfig[] =
{
	"skip SCE logo",
	"hide corrupt icon",
	"GAME folder homebrew",
	"Autorun program",
	"use NO-UMD",
	"fake region",
	"free UMD region"
};

static const u8 menu_baseconfig_s = 7;

static const char * menu_advanced[] =
{
	"plain modules in UMD/ISO",
	"excute BOOT.BIN in UMD/ISO",
	"always use isofs driver"
};

static const u8 menu_advanced_s = 3;

static const char * menu_registryhack[] =
{
	"swap button O/X",
	"activate WMA",
	"activate Flash Player"
};

static const u8 menu_registryhack_s = 3;

static const char * menu_extends[] =
{
	"CPU speed",
	"Plugins"
};

static const u8 menu_extends_s = 2;

static const char * menu_cpuspeed[] =
{
	"VSH",
	"GAME"
};

static u8 menu_cpuspeed_s = 2;

static const char * menu_plugin[] =
{
	"VSH",
	"GAME",
	"POPS"
};

static const u8 menu_plugin_s = 3;

static const char * menu_recovery[] =
{
	"Toggle USB",
	"Execute app",
	"File manager"
};

static const u8 menu_recovery_s = 3;

static const char * menu_toggleusb[] =
{
	"ms0:",
	"flash0:",
	"flash1:"
};

static const u8 menu_toggleusb_s = 3;

static const char * program_path = "ms0:/PSP/GAME/RECOVERY/EBOOT.PBP";

#endif
