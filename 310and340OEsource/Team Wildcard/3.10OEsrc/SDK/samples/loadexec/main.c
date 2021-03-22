#include <pspsdk.h>
#include <pspuser.h>
#include <pspctrl.h>
#include <systemctrl.h>
#include <systemctrl_se.h>

#include <string.h>

PSP_MODULE_INFO("BootLoader", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

int main_thread(SceSize args, void *argp)
{
	char file[256];
	struct SceKernelLoadExecVSHParam param;
	int apitype;

	if (argp)
	{
		char *p = argp+strlen((char *)argp)-10;
		*p = 0;
		sceIoChdir(argp);
	}

	pspDebugScreenInit();

	pspDebugScreenPrintf("Press X to loadexec EBOOT.BIN.\n");
	pspDebugScreenPrintf("Press O to loadexec the homebrew at ms0:/PSP/GAME/HOMEBREW/EBOOT.PBP (2.71 kernel)\n");
	pspDebugScreenPrintf("Press square to loadexec the homebrew at ms0:/PSP/GAME/HOMEBREW/EBOOT.PBP (1.50 kernel)\n");
	
	memset(&param, 0, sizeof(param));

	while (1)
	{
		SceCtrlData pad;

		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS)
		{
			strcpy(file, "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN");
			apitype = PSP_LOADMODULE_APITYPE_DISC; // for LoadExecVSHDisc
			break;
		}
		else if (pad.Buttons & PSP_CTRL_CIRCLE)
		{
			strcpy(file, "ms0:/PSP/GAME/HOMEBREW/EBOOT.PBP");
			apitype = PSP_LOADMODULE_APITYPE_MS2; // for LoadExecVSHMs2
			break;
		}
		else if (pad.Buttons & PSP_CTRL_SQUARE)
		{
			if (sctrlHENIsSE() && !sctrlHENIsDevhook())
			{
				sctrlSESetRebootType(1);
				apitype = PSP_LOADMODULE_APITYPE_MS2; // for LoadExecVSHMs2
				strcpy(file, "ms0:/PSP/GAME/HOMEBREW/EBOOT.PBP");
				break;
			}
			else
			{
				pspDebugScreenPrintf("\nThis operation can only be performed in SE without devhook running.\n");
				sceKernelDelayThread(200000);
			}
		}
	
		sceKernelDelayThread(10000);
	}

	pspDebugScreenPrintf("\nLoad executing. Wait...\n");

	if (!param.key)
		param.key = "game";

	param.size = sizeof(param);
	param.args = strlen(file)+1;
	param.argp = file;

	sctrlKernelLoadExecVSHWithApitype(apitype, file, &param);

	return sceKernelExitDeleteThread(0);
}

int module_start(SceSize args, void *argp)
{
	SceUID th = sceKernelCreateThread("main_thread", main_thread, 0x20, 0x10000, 0, NULL);

	if (th >= 0)
	{
		sceKernelStartThread(th, args, argp);
	}

	return 0;
}

