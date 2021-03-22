#include "common.h"

void copyfile(char *in, char *out)
{
	int input = -1, output = -1, bytesread = 1, byteswritten = 1;
	unsigned char inputbuffer[51200];

	input = sceIoOpen(in, PSP_O_RDONLY, 0777);
	output = sceIoOpen(out, PSP_O_CREAT | PSP_O_TRUNC | PSP_O_WRONLY, 0777);
		
	for(; bytesread > 0 && byteswritten == bytesread;) {
		bytesread = sceIoRead(input, inputbuffer, 51200);
		if(bytesread > 0) byteswritten = sceIoWrite(output, inputbuffer, bytesread);
	}
		
	sceIoClose(input); sceIoClose(output);
	printf("Copied %s\n", out);
}

void writefile(unsigned char *input, char *file, int len)
{
	int output;
	output = sceIoOpen(file, PSP_O_CREAT | PSP_O_TRUNC | PSP_O_WRONLY, 0777);
	sceIoWrite(output, input, len);
	sceIoClose(output);
	printf("Created %s (%i bytes)\n", file, len);
}

void deletefile(char *file)
{
	sceIoRemove(file);
	printf("Deleted %s\n", file);
}

void renamefile(char *old, char *nw)
{
	sceIoRename(old, nw);
	printf("Renamed %s\n", old);
}

unsigned int iserr(unsigned int val)
{
   return (val & 0x80000000);
}

// Get 0x######## error code
char *getError(unsigned int errornumber)
{
	static char err[10];
	sprintf(err, "0x%08X", errornumber);
	return err;
}

void pause_for_cross()
{
    SceCtrlData pad;
    u32 oldButtons = 0;

	for(;;) {
		sceCtrlReadBufferPositive(&pad, 1);
		if (oldButtons != pad.Buttons) {
			if (pad.Buttons & PSP_CTRL_CROSS) {
				break;
			} else if (pad.Buttons & PSP_CTRL_HOME) {
				sceKernelExitGame();
				break;
			}
		}

		oldButtons = pad.Buttons;
		sceDisplayWaitVblankStart();
	}
}

void pause_for_triangle()
{
    SceCtrlData pad;
    u32 oldButtons = 0;

	for(;;) {
		sceCtrlReadBufferPositive(&pad, 1);
		if (oldButtons != pad.Buttons) {
			if (pad.Buttons & PSP_CTRL_TRIANGLE) {
				break;
			} else if (pad.Buttons & PSP_CTRL_HOME) {
				sceKernelExitGame();
				break;
			}
		}

		oldButtons = pad.Buttons;
		sceDisplayWaitVblankStart();
	}
}

void verify_fail()
{
	printf("\nYour DATA.DXAR cannot be verified. The installation has stopped.\nPress X to quit.");
	pause_for_cross();
	sceKernelExitGame();
}

void reassign_fail()
{
	printf("\nThere was an error reassigning flash0 in read/write mode.\nThe installation has been stopped. Press X to quit.");
	pause_for_cross();
	sceKernelExitGame();
}

void folder_fail()
{
	printf("\nThere was an error making the needed folders in flash0.\nYour PSP will still be OK, but the installation has\nbeen stopped. Press X to quit.");
	pause_for_cross();
	sceKernelExitGame();
}
