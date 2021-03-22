#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>

#include "dxar.h"

u8 *dataOut, *dataOut2;

#define NUM_FILES 220
#define NUM_DIRS 14

enum {
	READ_MODE_DIRSONLY=0,
	READ_MODE_FILESONLY=1
} READ_MODE;

int is_prx(const char *file);
void add_file(const char * file, int sigcheck);
void add_dirs(const char * dir, int mode, int sigcheck);

void add_file(const char * file, int sigcheck) // convoluted method of deleting a file; copied from some other code
{
	int i=0, j;
	while(file[i] != '/') i++;
	j = strlen(file+i)+8;
	char *newname = malloc(j);
	memcpy(newname, "flash0:/", 7);
	newname[7]='\0';
	strcat(newname, file+i);

	printf("%s --> %s (sigcheck? %i)\n", file, newname, is_prx(file));
	dxarAddFileFromFile(file, newname, 1, is_prx(file) && sigcheck, dataOut, 0x8000000);
	free(newname);
}

int isdir (const char *path)
{
  struct stat stats;

  return stat (path, &stats) == 0 && S_ISDIR (stats.st_mode);
}


void add_dirs(const char * dir, int mode, int sigcheck) // recursively delete a directory, excluding flash0:/ itself
{
	DIR *dl = opendir(dir);
	if(!dl)
		return;
	struct dirent *sid;

	while(sid = readdir(dl))
	{
		if(sid->d_name[0] == '.') continue;
		char compPath[260];
		sprintf(compPath, "%s/%s", dir, sid->d_name);
		//if(FIO_S_ISDIR(sid->d_stat.st_mode))
		//{
		//	continue;
		//}
		if(isdir(compPath))
		{
			int i=0, j;
			while(compPath[i] != '/') i++;
			j = strlen(compPath+i)+8;
			char *newname = malloc(j);
			memcpy(newname, "flash0:/", 7);
			newname[7]='\0';
			strcat(newname, compPath+i);
			printf("%s --> %s\n", compPath, newname);
			if(mode != READ_MODE_FILESONLY)
				dxarAddDirectory(newname);

			free(newname);
			add_dirs(compPath, mode, sigcheck);
		} else {
			if(mode == READ_MODE_FILESONLY) add_file(compPath, sigcheck);
		}
		memset(&sid, 0, sizeof(sid));
	}
	closedir(dl);
}

int is_prx(const char *file)
{
	return strcmp(file+strlen(file)-3, "prx") ? 0 : 1;
}

int main(int argc, char **argv)
{
	if(argc < 2 || argc %3 != 1)
	{
		printf("usage: %s <section ID 1> <section 1 path> <section ID 2> <section 2 path> ...\n", argv[0]);
		return -1;
	}
	dataOut = (u8 *)memalign(0x40, 0x8000000);

	if (!dataOut) {printf("Cannot allocate memory (1).\n"); return 6000;}

	dataOut2 = (u8 *)memalign(0x40, 0x8000000);

	if (!dataOut2) {printf("Cannot allocate memory (2).\n"); return 6000;}

	int i, j;

	printf("init dxar: %i\n", dxarInit("DATA.DXAR"));
	printf("init dir section: %i\n", dxarInitSection("DIR"));
	add_dirs("flash0", 0, 0);
	printf("end dir section: %i\n", dxarEndSection(dataOut, 0x8000000));
	
	for(i=1;i<argc;i+=3)
	{
		printf("section %i, id: %s, path: %s, sigcheck: %i\n", i/3+1, argv[i], argv[i+1], argv[i+2][0]-0x30);

		printf("init %s section: %i\n", argv[i], dxarInitSection(argv[i]));
		add_dirs(argv[i+1], 1, argv[i+2][0]-0x30);
		printf("end %s section: %i\n", argv[i], dxarEndSection(dataOut, 0x8000000));	}

		printf("end: %i\n", dxarEnd(dataOut, 0x8000000));
	return 0;
}
