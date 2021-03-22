/*
gamecpu 0.3 
*/

#include "common.h"

#include "conf.h"

#define underclock_cpuspeed 111
#define underclock_busspeed 50
#define MAX_BTNS 23

PSP_MODULE_INFO("gamecpu_1", 0x1000, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

int font_init();
int blit_string(int sx, int sy, const char *msg, int bg);
SceUID thid;
CONFIGFILE config;
int i;
char *button_names[17] = { "SELECT", "START", "UP", "RIGHT", "DOWN", "LEFT", "LTRIGGER", "RTRIGGER", "TRIANGLE", "CIRCLE", "CROSS", "SQUARE", "HOME", "NOTE", "SCREEN", "VOLUP", "VOLDOWN" };
int button_codes[17] = { 0x000001, 0x000008, 0x000010, 0x000020, 0x000040, 0x000080, 0x000100, 0x000200, 0x001000, 0x002000, 0x004000, 0x008000, 0x010000, 0x800000, 0x400000, 0x100000, 0x200000 };

int messageTimer, delayTimer, checkSpeed;
int currentSpeedCpu, currentSpeedBus;
int realSpeedCpu, realSpeedBus;
char debugMessage[200];
char debugMessageReal[200];
int underclockTimer, underclocked;
int triggerRealButton1, triggerRealButton2;

int correctSpeed(int target, int actual) {
	if ((actual == (target + 1)) || (actual == (target - 1))) return target;
	else return actual;
}

int getRealButtonName (char *buttonname) {
	i = 0;
	while (i < 17) {
		if (strcmp(buttonname, button_names[i]) == 0) return button_codes[i];
		i++;
	}
	return 0x000001;
}

int getSqrRadius (int xcoord, int ycoord) {
	return ((xcoord + ycoord)^2);
}

void setClock (int cpuclk, int busclk) {
	scePowerSetClockFrequency(cpuclk, cpuclk, busclk);
}

void clearVariables (void) {
	debugMessage[0] = '\0';
	messageTimer = 5 * 60;
	delayTimer = 1 * 60;
}

int mainThread (SceSize args, void *argp) {
	SceCtrlData pad;
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

	read_config("ms0:/gamecpu_cfg.txt", &config);

	currentSpeedCpu = ((config.defaultCpuSpeed > 0) ? config.defaultCpuSpeed : 222);
	currentSpeedBus = ((config.defaultBusSpeed > 0) ? config.defaultBusSpeed : 111);
	setClock(currentSpeedCpu, currentSpeedBus);
	realSpeedCpu = correctSpeed(currentSpeedCpu, scePowerGetCpuClockFrequency());
	realSpeedBus = correctSpeed(currentSpeedBus, scePowerGetBusClockFrequency());
	checkSpeed = realSpeedCpu;
	
	triggerRealButton1 = getRealButtonName(config.triggerButton1);
	triggerRealButton2 = getRealButtonName(config.triggerButton2);

	if (config.autoUnderclock) {
		underclockTimer = config.autoUnderclock_timeout * 60;
		underclocked = 0;
	}

	debugMessage[0] = '\0';
	sprintf(debugMessage, "%s %i %s %i", "gamecpu 0.3 loaded -- cpu", realSpeedCpu, "/ bus", realSpeedBus);
	messageTimer = 10 * 60;

	while(1) {
		sceDisplayWaitVblankStart();

		if (delayTimer > 0) { delayTimer--; }

		if (messageTimer > 0) {
			//font_init();
			debugMessageReal[0] = '\0';
			sprintf(debugMessageReal, "%s - %i", debugMessage, ((messageTimer / 60) + 1));
			//sprintf(debugMessageReal, "%i", analogRadius);
			blit_string(0, 264, debugMessageReal, 1);
			messageTimer--;
		}
		
		sceCtrlPeekBufferPositive(&pad, 1);
		int analogRadius = getSqrRadius((pad.Ly - 128), (pad.Lx - 128));

		if (config.autoUnderclock) {
			if ((pad.Buttons == 50331648) && (analogRadius <= 30) && (analogRadius >= -30)) {
				if (underclockTimer > 0) underclockTimer--;
				if ((underclockTimer == 0) && (underclocked == 0)) {
					underclocked = 1;
					clearVariables();
					setClock(underclock_cpuspeed, underclock_busspeed);
					realSpeedCpu = correctSpeed(underclock_cpuspeed, scePowerGetCpuClockFrequency());
					realSpeedBus = correctSpeed(underclock_busspeed, scePowerGetBusClockFrequency());
					sprintf(debugMessage, "%s %i %s %i", "inactivity underclock -- cpu", realSpeedCpu, "/ bus", realSpeedBus);
				}
			}
			else {
				underclockTimer = config.autoUnderclock_timeout * 60;
				if (underclocked == 1) {
					underclocked = 0;
					goto setclock;
				}
			}
		}

		if (((pad.Buttons & (triggerRealButton1|triggerRealButton2)) == (triggerRealButton1|triggerRealButton2)) && (delayTimer == 0)) {
			checkSpeed = currentSpeedCpu;
			switch (checkSpeed) {
				case 222:
					currentSpeedCpu = 266;
					currentSpeedBus = 133;
					break;
				case 266:
					currentSpeedCpu = 333;
					currentSpeedBus = 166;
					break;
				case 333:
					currentSpeedCpu = 111;
					currentSpeedBus = 50;
					break;
				case 111:
					currentSpeedCpu = 166;
					currentSpeedBus = 80;
					break;
				case 166:
					currentSpeedCpu = 222;
					currentSpeedBus = 111;
					break;
			}
			setclock:
			clearVariables();
			setClock(currentSpeedCpu, currentSpeedBus);

			realSpeedCpu = correctSpeed(currentSpeedCpu, scePowerGetCpuClockFrequency());
			realSpeedBus = correctSpeed(currentSpeedBus, scePowerGetBusClockFrequency());
			sprintf(debugMessage, "%s %i %s %i", "cpu", realSpeedCpu, "/ bus", realSpeedBus);
		}
	}
	return 0;
}

int module_start (SceSize args, void *argp) {
	thid = sceKernelCreateThread("gamecpu_2", mainThread, 0x32, 0x1000, 0, NULL);
	if (thid >= 0)
		sceKernelStartThread(thid, args, argp);

	return 0;
}

int module_stop (void) {
	return 0;
}
