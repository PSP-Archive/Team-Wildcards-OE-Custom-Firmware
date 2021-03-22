#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspaudiolib.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "oggplayer.h"
#include <mikmod.h>

PSP_MODULE_INFO("mediamodule", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

void play_ogg(char *file);
int exists(char *file);
void get_fileext(char *file, char *ext);
int is_mikmodfile(char *ext);

#define printf	pspDebugScreenPrintf
/* Exit callback */
int exitCallback(int arg1, int arg2, void *common) {
	sceKernelExitGame();
	return 0;
}

/* Callback thread */
int callbackThread(SceSize args, void *argp) {
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", (void*) exitCallback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int setupCallbacks(void) {
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", callbackThread, 0x11, 0xFA0, 0, 0);
	if (thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	}
	return thid;
}

int exists(char *file)
{
	int x;
	if((x=sceIoOpen(file, PSP_O_RDONLY, 0777))>=0) return (sceIoClose(x) ? 1 : 1);
	else return 0;
}

void get_fileext(char *file, char *ext)
{
	char *f=file;
	
	while(strstr(f, ".")) f=strstr(f, ".")+1;
	strcpy(ext,f);
}

#define NUM_MIKMODFORMATS 19
char *mikmodformats[NUM_MIKMODFORMATS] =
{
	"mod",
	"669",
	"amf",
	"apun",
	"dsm",
	"far",
	"gdm",
	"it",
	"imf",
	"mod",
	"med",
	"mtm",
	"okt",
	"s3m",
	"stm",
	"stx",
	"ult",
	"uni",
	"xm"
};

int is_mikmodfile(char *ext)
{
	int x, y;
	for(x=0;x<NUM_MIKMODFORMATS;x++)
		for(y=0;y<strlen(mikmodformats[x]);y++)
			if(ext[y]==mikmodformats[x][y] || ext[y]==mikmodformats[x][y]-32) return 1;

	return 0;
}

static int oldButtons = 0;
int changedButtons=0;
SceCtrlData pad;

void play_mikmod(char *file)
{
	if(!exists(file))
	{
		printf("%s not found!", file);
		return;
	}
	printf("Playing %s\n", file);
	FILE *modfile = fopen(file,"rb");
	MODULE *mod = Player_LoadFP(modfile, 64, 0);
	if(mod)
	{
		printf("play/pause: start\nstop: cross\nprevious/next position: d-pad left/right\nstart over: square\n\n");
		if(mod->songname) printf("Song Title: %s\n", mod->songname);
		if(mod->comment) printf("Song Comment: %s\n", mod->comment);
		Player_Start(mod);
		while(Player_Active() && !(changedButtons & PSP_CTRL_CROSS)) {
			sceCtrlReadBufferPositive(&pad, 1);
			sceDisplayWaitVblankStart();
			pspDebugScreenSetXY(0, 2);
			printf("Current position: %i of %i              ", mod->sngpos, mod->numpos);
			changedButtons = pad.Buttons & (~oldButtons);
			MikMod_Update();
			if (changedButtons & PSP_CTRL_START)
				Player_TogglePause();
			else if(changedButtons & PSP_CTRL_SQUARE)
				Player_SetPosition(mod->reppos);
			else if(changedButtons & PSP_CTRL_LEFT)
				Player_PrevPosition();
			else if(changedButtons & PSP_CTRL_RIGHT)
				Player_NextPosition();
			oldButtons = pad.Buttons;
		}
		Player_Stop();
		Player_Free(mod);
	} else
		printf("Error loading module!");

}

int was_ogg = 0;

void play_ogg(char *file)
{
	char ogginfo[1024];
	if(!exists(file))
	{
		printf("\n%s not found!", file);
		return;
	}
	printf("Playing %s\n", file);
	OGG_Init(1);
	OGG_Load(file);
	memset(ogginfo, 0, 1024);
	OGG_GetInfo(ogginfo);
	pspDebugScreenSetXY(0, 29);
	printf(ogginfo);
	OGG_Play();
	was_ogg = 1;
	pspDebugScreenSetXY(0, 2);
	printf("play/pause: start\nstop: cross\nrewind/fast forward: d-pad left/right\nstart over: square");
	while(!OGG_EndOfStream() && !(changedButtons & PSP_CTRL_CROSS))
	{
		pspDebugScreenSetXY(0, 1);
		memset(ogginfo, 0, 1024);
		OGG_GetTimeString(ogginfo);
		printf("%s       seekable: %i", ogginfo, OGG_IsSeekable());
		sceCtrlReadBufferPositive(&pad, 1);
		sceDisplayWaitVblankStart();
		changedButtons = pad.Buttons & (~oldButtons);
		if (changedButtons & PSP_CTRL_START)
			OGG_Pause();
		else if ((changedButtons & PSP_CTRL_LEFT) && OGG_IsSeekable())
			OGG_Rewind();
		else if((changedButtons & PSP_CTRL_RIGHT) && OGG_IsSeekable())
			OGG_FastForward();
		else if(changedButtons & PSP_CTRL_SQUARE)
			OGG_Restart();
		oldButtons = pad.Buttons;
	}
	OGG_Stop();
	OGG_FreeTune();
}

int main(int argc, char *argv[]) {
	setupCallbacks();
	pspDebugScreenInit();
	pspAudioInit();
	MikMod_RegisterAllDrivers();
	MikMod_RegisterAllLoaders();
	MikMod_Init("");
	char *file;
	char ext[6];
	if(argc == 2) file = argv[1];
	else
	{
		pspAudioEnd();
		sceKernelExitGame();
	}
	if(argc == 2)
	{
		if(exists(file))
		{
			get_fileext(file, ext);
			if(!strcmp(ext,"ogg"))
				play_ogg(file);
			else if(is_mikmodfile(ext))
				play_mikmod(file);
			else
				printf("Unknown format: %s, %s!", file, ext);
		} else
			printf("%s not found!", file);

	} else
		printf("No file specified!");

	MikMod_Exit();
	if(was_ogg) OGG_End();
	pspAudioEnd();
	sceKernelExitGame();
	return 0;
}
