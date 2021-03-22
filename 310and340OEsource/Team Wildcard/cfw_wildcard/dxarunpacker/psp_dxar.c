#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <pspvshbridge.h>
#include "dxar.h"
#include "common.h"
#include "dxar_info.h"

int dxar_index;

/*void get_dxar_version(SceUID dxar);
void check_dxar();
void check_sha1(unsigned char *, unsigned char *, int len);
void print_sha1(unsigned char *);
void del_file(const char * file);
void del_dir(const char * dir);*/
void get_header(dxarfile *dxar, SceUID fptr);
void get_section_headers(dxarfile *dxar, SceUID fptr);
void write_section(dxarfile *dxar, int index, SceUID fptr);
void write_file_from_dxar(dxarfile *dxar, dxar_file_entry *file, int sectionindex, int num, int sectionfilecount);
void setup_paths(dxarfile *dxar);
//void reassign_flash0();
void free_dxar(dxarfile *dxar);
//void wipeflash();
void get_dir_section(dxarfile *dxar, SceUID fptr);
extern void getcheck(unsigned char *block, int len, unsigned char *check);
extern int GenerateSigCheck(u8 *buf);
extern u32 FindProc(const char* szMod, const char* szLib, u32 nid);

/*void get_dxar_version(SceUID dxar)
{
	int x, size;
	sceIoLseek(dxar, 8, PSP_SEEK_SET);
	sceIoRead(dxar, &size, sizeof(int));
	size += 0xD0;
	for(x=0;x<NUM_CFWS;x++)
	{
		if(dxar_sizes[x] == size)
		{
			dxar_index = x;
			return;
		}
	}
	printf("Unknown DXAR! (0x%08X)\n", size);
	verify_fail();

}

void check_dxar()
{
	SceUID dxar = sceIoOpen("DATA.DXAR", PSP_O_RDONLY, 0777);
	get_dxar_version(dxar);
	unsigned char *)malloc(dxar_sizes[dxar_index]*sizeof(unsigned char));
	printf("\nVerifying DXAR contents for %s...", dxar_names[dxar_index]);
	if(iserr(dxar))
		{
		free(dxar_contents);
		printf(" failed (DATA.DXAR not found)\n");
		pause_for_cross();
		pause_for_cross();
		verify_fail();
	}
	sceIoClose(dxar);
	dxar = sceIoOpen("DATA.DXAR", PSP_O_RDONLY, 0777);
	sceIoRead(dxar, dxar_contents, dxar_sizes[dxar_index]*sizeof(unsigned char));
	check_sha1(dxar_contents, dxar_sha1s[dxar_index], dxar_sizes[dxar_index]);
	free(dxar_contents);
	printf(" OK\n");
}

void check_sha1(unsigned char *buffer, unsigned char *expected, int len)
{
	unsigned char *check;
	unsigned int fd;
	check = (unsigned char *)malloc(20*sizeof(unsigned char));
	fd = sceIoOpen("DATA.DXAR", PSP_O_RDONLY, 0777);
	if(fd == 0x80010002)
	{
		free(check);
		printf(" failed (DATA.DXAR not found)\n");
		verify_fail();
	}
	sceIoClose(fd);
	getcheck(buffer, len, check);
	if(memcmp(expected, check, 20))
	{
		free(check);
		printf(" failed (SHA1 mismatch)\n");
		verify_fail();
	}
	free(check);
}

void print_sha1(unsigned char *sha1)
{
	int i;
	for(i=0;i<19;i++)
		printf("0x%02X,", sha1[i]);
	printf("0x%02X\n", sha1[19]);
}

void del_file(const char * file) // convoluted method of deleting a file; copied from some other code
{
	pspDebugScreenSetXY(19, 9);
	printf("%s                                            ", file);
	SceIoStat stat;
	memset(&stat, 0, sizeof(SceIoStat));
	sceIoGetstat(file, &stat);
	stat.st_attr &= ~0x0F;
	sceIoChstat(file, &stat, 3);
	sceIoRemove(file);
}

void del_dir(const char * dir) // recursively delete a directory, excluding flash0:/ itself
{
	pspDebugScreenSetXY(19, 9);
	printf("%s                                            ", dir);
	int dl = sceIoDopen(dir);
	if(dl < 0)
		return;
	SceIoDirent sid;
	memset(&sid, 0, sizeof(SceIoDirent));
	while(sceIoDread(dl, &sid))
	{
		if(sid.d_name[0] == '.') continue;
		char compPath[260];
		sprintf(compPath, "%s/%s", dir, sid.d_name);
		if(FIO_S_ISDIR(sid.d_stat.st_mode))
		{
			del_dir(compPath);
			continue;
		}
		del_file(compPath);
		memset(&sid, 0, sizeof(SceIoDirent));
	}
	sceIoDclose(dl);
	if(strcmp(dir, "flash0:/")) sceIoRmdir(dir);
}*/

void write_oe() { // get the necessary metainfo, set up the paths, and write the sections
	dxarfile *data_dxar = (dxarfile *)malloc(sizeof(dxarfile));
	
	data_dxar->header = (dxar_header*)malloc(sizeof(dxar_header));
	
	SceUID dxar = sceIoOpen("DATA.DXAR", PSP_O_RDONLY, 0777);

	get_header(data_dxar, dxar);
	get_section_headers(data_dxar, dxar);
	get_dir_section(data_dxar, dxar);

	printf("Press Triangle to begin unpacking the DXAR.\nIf you wish to quit, press Home.\n");
	pause_for_triangle();
	//printf("Writing new flash0 contents in ten seconds\nTHIS IS YOUR LAST CHANCE TO BAIL OUT!\n!!DO NOT REMOVE YOUR MS OR TURN OFF YOUR PSP OR YOU WILL BRICK!!\n");
	//sceKernelDelayThread(10*1000*1000);

	setup_paths(data_dxar);

	int x;
	printf("found %i sections\n", data_dxar->header->num_sections);
	for(x=1; x < data_dxar->header->num_sections; x++) // skip the first section (DIR)
		write_section(data_dxar, x, dxar);

	sceIoClose(dxar);
	
	free_dxar(data_dxar);
}

void get_header(dxarfile *dxar, SceUID fptr)
{
	int x;
	sceIoRead(fptr, dxar->header, sizeof(dxar_header));
	dxar->header->section_offsets = (int *)malloc(dxar->header->num_sections*sizeof(int)); // allocate memory for the list of section offsets
	sceIoLseek(fptr, 0x10, PSP_SEEK_SET);
	for(x=0; x < dxar->header->num_sections; x++)
		sceIoRead(fptr, &(dxar->header->section_offsets[x]), sizeof(int)); // get the section offsets
}

void get_section_headers(dxarfile *dxar, SceUID fptr)
{
	int x;

	dxar->dir_section = (dxar_dir_section*)malloc(sizeof(dxar_dir_section)); // allocate memory for the directory section

	sceIoLseek(fptr, dxar->header->section_offsets[0], PSP_SEEK_SET);
	sceIoRead(fptr, &(dxar->dir_section->header), sizeof(dxar_section_header)); // read the directory section header

	dxar->file_sections = (dxar_file_section**)malloc(dxar->header->num_sections*sizeof(dxar_file_section *)); // allocate memory for the list of file sections

	for(x=0; x < dxar->header->num_sections-1; x++) // for each file section...
	{
		dxar->file_sections[x] = (dxar_file_section *)malloc(sizeof(dxar_file_section)); // allocate memory for the section header

		sceIoLseek(fptr, dxar->header->section_offsets[x+1], PSP_SEEK_SET); // skip ahead in the file to this section

		sceIoRead(fptr, &(dxar->file_sections[x]->header), sizeof(dxar_section_header)); // read the header
	}

}

void get_dir_section(dxarfile *dxar, SceUID fptr)
{
	int x;
	dxar->dir_section->dirs = (dxar_dir_entry **)malloc(dxar->dir_section->header.num_entries * sizeof(dxar_dir_entry *)); // allocate memory for the list of directory entries

	sceIoLseek(fptr, dxar->header->section_offsets[0] + sizeof(dxar_section_header), PSP_SEEK_SET);
	for(x=0; x < dxar->dir_section->header.num_entries; x++) // for each directory entry...
	{
		dxar->dir_section->dirs[x] = (dxar_dir_entry *)malloc(sizeof(dxar_dir_entry)); // allocate memory for it
		sceIoRead(fptr, dxar->dir_section->dirs[x], sizeof(dxar_dir_entry)); // read the entry
	}
}

void write_section(dxarfile *dxar, int index, SceUID fptr)
{
	int x;

	int file_index = index - 1;
	pspDebugScreenSetXY(0, 3+file_index);
	printf("writing section %s - ", dxar->file_sections[file_index]->header.name);
	// for each file section...
	dxar->file_sections[file_index]->files = (dxar_file_entry **)malloc(dxar->file_sections[file_index]->header.num_entries*sizeof(dxar_file_entry *)); // allocate memory for the list of file entries
	sceIoLseek(fptr, dxar->header->section_offsets[index] + sizeof(dxar_section_header), PSP_SEEK_SET); // skip to the first file entry in this section
	for(x=0; x < dxar->file_sections[file_index]->header.num_entries; x++) // for every entry in this section...
	{
		dxar->file_sections[file_index]->files[x] = (dxar_file_entry *)malloc(sizeof(dxar_file_entry)); // allocate space for the entry
		sceIoRead(fptr, dxar->file_sections[file_index]->files[x], sizeof(dxar_file_entry)); // read the entry
		int filesize = (dxar->file_sections[file_index]->files[x]->is_deflated ? dxar->file_sections[file_index]->files[x]->deflated_size : dxar->file_sections[file_index]->files[x]->inflated_size); // get stored file size
		dxar->file_sections[file_index]->files[x]->data = (unsigned char *)malloc(filesize*sizeof(unsigned char)); // allocate space for the file data
		sceIoLseek(fptr, -8, PSP_SEEK_CUR);
		sceIoRead(fptr, dxar->file_sections[file_index]->files[x]->data, filesize*sizeof(unsigned char)); // read file data into memory
		write_file_from_dxar(dxar, dxar->file_sections[file_index]->files[x], file_index, x, dxar->file_sections[file_index]->header.num_entries);
		free(dxar->file_sections[file_index]->files[x]->data); // free up some memory
		free(dxar->file_sections[file_index]->files[x]); // free up some more memory
	}
	free(dxar->file_sections[file_index]->files);
}

void write_file_from_dxar(dxarfile *dxar, dxar_file_entry *file, int sectionindex, int num, int sectionfilecount)
{
	memcpy(file->path, "ms0:/f0", 7);
	int	filesize = file->inflated_size;
	int needs_inflate = file->is_deflated;

	SceUID out = sceIoOpen(file->path, PSP_O_WRONLY | PSP_O_CREAT, 0777);
	// display progress percentage and current file being written
	pspDebugScreenSetXY(19+strlen(dxar->file_sections[sectionindex]->header.name), 3+sectionindex);
	printf("%i%%                                                             ", (100*(num+1))/sectionfilecount);
	pspDebugScreenSetXY(0, 4+sectionindex);
	printf("writing file %s (%i bytes)                                                             \n", file->path, filesize);
	if(!needs_inflate)
	{
		if(file->needs_sigcheck)
			GenerateSigCheck((u8 *)file->data);

		sceIoWrite(out, file->data, filesize);
	} else {
		int internal_filesize = file->deflated_size;

		// thank GOD for flatwhatson, or I'd still be stuck on zlib
		z_stream z;
		// Set up the zlib inflation
		z.zalloc = Z_NULL;
		z.zfree = Z_NULL;
		z.opaque = Z_NULL;
		z.avail_in = 0;
		z.next_in = Z_NULL;
		inflateInit2(&z, -15);
		// Set up the input and output streams
		z.avail_in = internal_filesize;
		z.next_in = file->data;
		z.avail_out = filesize;
		void *output = malloc(filesize);
		z.next_out = (Bytef *)output;

		// et voila.
		inflate(&z, Z_NO_FLUSH);
		int inflate_ok = inflateEnd(&z);
		if(inflate_ok)
		{
			//error - can't handle it; you're fucked if this doesn't work and flash0 was erased :/
		} else {
			if(file->needs_sigcheck)
				GenerateSigCheck((u8 *)output);
				sceIoWrite(out, (unsigned char *)output, filesize);
		}
	}
	sceIoClose(out);
}

/*void reassign_flash0()
{
	SceUID res = sceIoUnassign("flash0:");
	if(res)
	{
		printf("Unassigning flash0 failed: %s\n", getError(res));
		reassign_fail();
	}

	res = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, 0, 0);
	if(res)
	{
		printf("Reassigning flash0 failed: %s\n", getError(res));
		reassign_fail();
	}
}*/

void setup_paths(dxarfile *dxar)
{
	// create the directory structure on flash0
	/*reassign_flash0();

	printf("Erasing flash0:... ");
	
	//wipeflash();
	printf("done.\n");*/

	// write to flash0
	int x;
	SceUID res;

	printf("Setting up directory structure... ");
	sceIoMkdir("ms0:/f0", 0777);
	for(x=0; x < dxar->dir_section->header.num_entries; x++)
	{
		char *path = (char *)malloc(strlen(dxar->dir_section->dirs[x]->path)+1);
		strcpy(path, dxar->dir_section->dirs[x]->path);
		memcpy(path, "ms0:/f0", 7);

		res = sceIoMkdir(path, 0777);
		if(res)
		{
			//printf("%s\n", getError(res));
			if((unsigned int)res == 0x80010002)
			{
				printf("Couldn't make folder %s!\n", path);
				folder_fail();
			}
		}
		free(path);
	}
	printf("Done.\n");
}

void free_dxar(dxarfile *dxar)
{
	int x;
	for(x=0; x < dxar->dir_section->header.num_entries; x++) // free the directory entries
	{
		free(dxar->dir_section->dirs[x]);
	}

	free(dxar->file_sections); // free the file entry list
	free(dxar->dir_section); // free the directory entry list
	free(dxar->header->section_offsets); // free the section offset list
	free(dxar->header); // free the header
	free(dxar); // free the dxar :)
}

/*void wipeflash()
{
	del_dir("flash0:/");
	printf(" done.                                                                                        \n");
}*/
