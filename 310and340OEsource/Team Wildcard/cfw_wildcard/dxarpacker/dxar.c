#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include "dxar.h"
#include "sha1.h"

DXAR_Header header;
DXAR_Section section;
FILE*	fd;
int		inited=0;
int		insection;
char	filename[256];


int dxarInit(const char *name)
{
	if (inited)
		return -1;

	strncpy(filename, name, 256);
	
	fd = fopen(filename, "wb+");

	if (!fd)
		return -1;

	memset(&header, 0, sizeof(DXAR_Header));

	header.magic = DXAR_MAGIC;
	header.version = 1;

	if (fwrite(&header, 1, sizeof(header), fd) < sizeof(header))
		return -1;

	insection = 0;
	inited = 1;	
	
	return 0;
}

int dxarInitSection(char *sectname)
{
	if (!inited)
		return -1;

	if (insection)
		return -1;

	if (strlen(sectname) > 31)
		return -1;

	memset(&section, 0, sizeof(section));
	strcpy(section.sectionname, sectname);

	header.sections[header.nsections] = ftell(fd);
	
	if (fwrite(&section, 1, sizeof(section), fd) < sizeof(section))
		return -1;

	insection = 1;

	return 0;
}

z_stream z;

int deflateCompress(void *inbuf, int insize, void *outbuf, int outsize)
{
	int res;
	
	z.zalloc = Z_NULL;
	z.zfree  = Z_NULL;
	z.opaque = Z_NULL;

	if (deflateInit2(&z, Z_DEFAULT_COMPRESSION , Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK)
		return -1;

	z.next_out  = outbuf;
	z.avail_out = outsize;
	z.next_in   = inbuf;
	z.avail_in  = insize;

	if (deflate(&z, Z_FINISH) != Z_STREAM_END)
	{
		return -1;
	}

	res = outsize - z.avail_out;

	if (deflateEnd(&z) != Z_OK)
	{
		printf("deflate failed\n");
		return -1;
	}

	return res;
}

int dxarAddFileFromFile(char *infile, char *internalname, int forcecompress, int sigcheck, void *buf2, int buf2size)
{
	void *data;
	FILE *f = fopen(infile, "rb");
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	data = malloc(len);
	fseek(f, 0, SEEK_SET);
	fread(data, 1, len, f);
	fclose(f);
	int i = dxarAddFile(internalname, data, len, forcecompress, sigcheck, buf2, buf2size);
	free(data);
	return i;
}

int	dxarAddFile(char *file, void *buf, int size, int forcecompress, int sigcheck, void *buf2, int buf2size)
{
	DXAR_FileEntry entry;
	int	writeplain = 0;
	
	if (!inited)
		return -1;

	if (!insection)
		return -1;

	if (strlen(file) > 127)
		return -1;

	if (!buf || !buf2)
		return -1;

	memset(&entry, 0, sizeof(entry));
	strcpy(entry.filepath, file);
	
	entry.filesize = size;
	entry.sigcheck = sigcheck;

	/*if (sceKernelUtilsMd5Digest(buf, size, entry.md5) < 0)
	{
		return -1;
	}*/

	if (!forcecompress)
	{
		u32 *buf32 = (u32 *)buf;
		
		if (buf32[0] == 0x5053507E) // ~PSP
		{
			u16 *buf16 = (u16 *)buf;

			if (buf16[6/2] & 1) // already comprressed
				writeplain = 1;
		}
	}

	if (size < (20*1024))
		writeplain = 1;

	if (!writeplain)
	{
		int compsize = deflateCompress(buf, size, buf2, buf2size);

		if (compsize <= 0)
			return -1;
	
		if ((compsize >= size) && !forcecompress)
			writeplain = 1;
		else
		{
			entry.compfilesize = compsize;
			entry.compression = COMPRESSION_DEFLATE;
		}		
	}

	if (fwrite(&entry, 1, sizeof(entry), fd) < sizeof(entry))
		return -1;

	if (writeplain)
	{
		if (fwrite(buf, 1, size, fd) < size)
		{
			return -1;
		}
	}

	else
	{
		if (fwrite(buf2, 1, entry.compfilesize, fd) < entry.compfilesize)
		{
			return -1;
		}
	}

	section.nfiles++;

	return 0;
}

int	dxarAddDirectory(char *dir)
{
	DXAR_FileEntry entry;	
	
	if (!inited)
		return -1;

	if (!insection)
		return -1;

	if (strlen(dir) > 127)
		return -1;

	memset(&entry, 0, sizeof(entry));
	strcpy(entry.filepath, dir);

	entry.isdirectory = 1;

	if (fwrite(&entry, 1, sizeof(entry), fd) < sizeof(entry))
		return -1;	

	section.nfiles++;

	return 0;
}

int dxarEndSection(void *buf, int bufsize)
{
	struct sha_ctx ctx;
	int remaining;
	
	if (!inited)
		return -1;

	if (!insection)
		return -1;

	section.sectionsize = ftell(fd) - header.sections[header.nsections] - sizeof(section);
	remaining = section.sectionsize;

	fseek(fd, header.sections[header.nsections]+sizeof(section), SEEK_SET);

	sha_init(&ctx);
	
	while (remaining > 0)
	{
		int blocksize;

		if (remaining > bufsize)
			blocksize = bufsize;
		else
			blocksize = remaining;

		fread(buf, 1, blocksize, fd);
		sha_update(&ctx, buf, blocksize);
		remaining -= blocksize;
	}

	fseek(fd, header.sections[header.nsections], SEEK_SET);
	sha_final(&ctx); sha_digest(&ctx, section.sha1);

	if (fwrite(&section, 1, sizeof(section), fd) < sizeof(section))
		return -1;

	fseek(fd, header.sections[header.nsections]+sizeof(section)+section.sectionsize, SEEK_SET);

	header.nsections++;	
	insection = 0;

	return 0;
}

int dxarEnd(void *buf, int bufsize)
{
	
	struct sha_ctx ctx;
	int remaining;
	
	if (!inited)
		return -1;

	if (insection)
		return -1;

	fclose(fd);

	fd = fopen(filename, "rb+");

	if (!fd)
		return -1;
	fseek(fd, 0, SEEK_END);
	header.size = ftell(fd) - sizeof(header);
	remaining = header.size;
	fseek(fd, sizeof(header), SEEK_SET);

	sha_init(&ctx);
	
	while (remaining > 0)
	{
		int blocksize;

		if (remaining > bufsize)
			blocksize = bufsize;
		else
			blocksize = remaining;

		fread(buf, 1, blocksize, fd);
		sha_update(&ctx, buf, blocksize);
		remaining -= blocksize;
	}

	sha_final(&ctx); sha_digest(&ctx, header.sha1);
	fseek(fd, 0, SEEK_SET);

	if (fwrite(&header, 1, sizeof(header), fd) < sizeof(header))
		return -1;

	fclose(fd);
	inited = 0;

	return 0;

}
