#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspaudiolib.h>
#include <psppower.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "aacplayer.h"
#include "mp3player.h"
#include "oggplayer.h"
#include "mikmod.h"

PSP_MODULE_INFO("mediamodule", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

void play_aac(char *file);
void play_mikmod(char *file);
void play_mp3(char *file);
void play_ogg(char *file);
int exists(char *file);
void get_fileext(char *file, char *ext);
int strmatch_nocase(char *a, char *b);
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

extern int tolower(int c);

int strmatch_nocase(char *a, char *b)
{
	if(strlen(a) != strlen(b)) return 0;
	int x;
	for(x=0;x<strlen(a);x++)
		if(tolower(a[x]) != tolower(b[x])) return 0;

	return 1;
}

int is_mikmodfile(char *ext)
{
	int x;
	int ret=0;
	for(x=0;x<NUM_MIKMODFORMATS;x++)
		if(strmatch_nocase(ext, mikmodformats[x])) ret=1;

	return ret;
}

static int oldButtons = 0;
int changedButtons=0;
SceCtrlData pad;
int was_aac = 0, was_mp3 = 0, was_ogg = 0;

void play_aac(char *file)
{
  	if(!exists(file))
	{
		printf("%s not found!", file);
		return;
	}

	printf("Playing %s\n", file);
	AAC_Init(1);
	AAC_Load(file);
	AAC_Play();
	printf("play/pause: start\nstop: cross\n\n");
	while(!AAC_EndOfStream() && !(changedButtons & PSP_CTRL_CROSS))
	{
		scePowerTick(0);
		sceDisplayWaitVblankStart();
		sceCtrlReadBufferPositive(&pad, 1);
		changedButtons = pad.Buttons & (~oldButtons);
		if (changedButtons & PSP_CTRL_START)
			AAC_Pause();
		oldButtons = pad.Buttons;
	}
	AAC_Stop();
}

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
			scePowerTick(0);
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

void play_mp3(char *file)
{
	char mp3info[1024];
	if(!exists(file))
	{
		printf("%s not found!", file);
		return;
	}
	printf("Playing %s\n", file);
	MP3_Init(1);
	MP3_Load(file);
	MP3_Play();
	printf("play/pause: start\nstop: cross\n\n");
	while(!MP3_EndOfStream() && !(changedButtons & PSP_CTRL_CROSS))
	{
		scePowerTick(0);
		sceDisplayWaitVblankStart();
		sceCtrlReadBufferPositive(&pad, 1);
		memset(mp3info, 0, 1024);
		MP3_GetTimeString(mp3info);
		pspDebugScreenSetXY(0, 2);
		printf("%s   ", mp3info);
		changedButtons = pad.Buttons & (~oldButtons);
		if (changedButtons & PSP_CTRL_START)
			MP3_Pause();
		oldButtons = pad.Buttons;
	}
	MP3_Stop();
	MP3_FreeTune();
}

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
		scePowerTick(0);
		pspDebugScreenSetXY(0, 1);
		memset(ogginfo, 0, 1024);
		OGG_GetTimeString(ogginfo);
		printf("%s   seekable: %i", ogginfo, OGG_IsSeekable());
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
	//scePowerSetClockFrequency(333, 333, 166);
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
			if(strmatch_nocase(ext,"ogg"))
				play_ogg(file);
			else if(strmatch_nocase(ext,"mp3"))
				play_mp3(file);
			else if(strmatch_nocase(ext,"aac") || strmatch_nocase(ext,"mp4") || strmatch_nocase(ext,"m4a") || strmatch_nocase(ext,"m4p") || strmatch_nocase(ext,"3gp"))
				play_aac(file);
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
	if(was_mp3) MP3_End();
	if(was_aac) AAC_End();
	pspAudioEnd();
	sceKernelExitGame();
	return 0;
}
