#include <zlib.h>

typedef struct {
	char magic[4];
	int version;
	int remaining_size;
	int num_sections;
	int *section_offsets;
} dxar_header;

typedef struct {
	char name[32];
	int length;
	int num_entries;
	unsigned char sha1[0x14];
	unsigned char unknown2[0x34];
} dxar_section_header;

typedef struct {
	char path[0xC4];
} dxar_dir_entry;

typedef struct {
	char path[0x7E];
	int inflated_size;
	int deflated_size;
	int is_deflated;
	int needs_sigcheck;
	unsigned char unknown1[0x6];
	unsigned char unknown2[0x10];
	unsigned char unknown3[0x20];
	unsigned char *data;
} dxar_file_entry;

typedef struct {
	dxar_section_header header;
	dxar_dir_entry **dirs;
} dxar_dir_section;

typedef struct {
	dxar_section_header header;
	dxar_file_entry **files;
} dxar_file_section;

typedef struct {
	dxar_header *header;
	dxar_dir_section *dir_section;
	dxar_file_section **file_sections;
} dxarfile;
