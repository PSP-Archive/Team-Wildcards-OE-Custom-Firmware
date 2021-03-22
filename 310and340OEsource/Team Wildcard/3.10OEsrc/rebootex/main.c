#include <pspsdk.h>

int Main(void *, void *, void *, void *);

int Reboot_Entry(void *a0, void *a1, void *a2, void *a3)
{
	return Main(a0, a1, a2, a3);
}

#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000

#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a); 

//#define CHANGE_FUNC(a, f) _sw(J_OPCODE | (((u32)(f) & 0x3FFFFFFF) >> 2), a); _sw(0, a+4);

int (* Real_Reboot)(void *a0, void *a1, void *a2, void *a3) = (void *)0x88600000;
int (* sceBootLfatOpen)(char *file) = (void *)0x88607904;
int (* DcacheClear310)(void) = (void *)0x88602734;
int (* IcacheClear310)(void) = (void *)0x88602100;
int (* DcacheClear150)(void) = (void *)0x88c02c64;
int (* IcacheClear150)(void) = (void *)0x88c02c90;
int (* sceKernelCheckExecFile)(void *buf, int *check);

#define XD 0x95

static const u8 key_data_310[0x10] = 
{
	0xa2^XD, 0x41^XD, 0xe8^XD, 0x39^XD, 0x66^XD, 0x5b^XD, 0xfa^XD, 0xbb^XD,
	0x1b^XD, 0x2d^XD, 0x6e^XD, 0x0e^XD, 0x33^XD, 0xe5^XD, 0xd7^XD, 0x3f^XD
};

static void buld_key_param(const u8 *key_real)
{
	int i;
	u8 *dst  = (u8 *)(0xbfc00200 - 0x35231452); // KEY param
	u8 *src  = (u8 *)(0x88611888 - 0x35231452); // KEY seed
	// key seed
	for(i=0;i<0x10;i++)
	{
		dst[0x35231452+i] = src[0x35231452+i] ^ key_real[i] ^ XD;
	}
}

int memcmp(u8 *m1, u8 *m2, int size)
{
	int i;

	for (i = 0; i < size; i++)
	{
		if (m1[i] != m2[i])
			return m2[i] - m1[i];
	}

	return 0;
}

void ClearCaches310()
{
	DcacheClear310();
	IcacheClear310();
}

void ClearCaches150()
{
	DcacheClear150();
	IcacheClear150();
}

int sceBootLfatOpenPatched(char *file)
{	
	if (memcmp(file, "/kd", 3) == 0)
	{
		file[2] = 'n';
	}

	else if (memcmp(file, "/vsh/module", 11) == 0)
	{
		file[5] = 'n';
	}

	return sceBootLfatOpen(file);
}

int sceKernelCheckExecFilePatched(void *buf, int *check)
{
	int res;
	int isPlain;
	int index;
	
	isPlain = (((u32 *)buf)[0] == 0x464C457F); /* ELF */		
		
	if (isPlain) 
	{
		if (check[0x44/4] != 0)
		{
			check[0x48/4] = 1;
			return 0;
		}
	}	

	res = sceKernelCheckExecFile(buf, check);

	if (isPlain)
	{
		index = check[0x4C/4];

		if (check[0x4C/4] < 0)
		{
			index += 3;
		}			

		if ((check[8/4] == 0x20) || 
			((check[8/4] > 0x20) && (check[8/4] < 0x52)))		
		{
			if ((((u32 *)buf)[index / 4] & 0x0000FF00))
			{			
				check[0x44/4] = 1;
				check[0x58/4] = ((u32 *)buf)[index / 4] & 0xFFFF;
				return 0;
			}			
		}		
	}
	
	return res;	
}

int PatchLoadCore310(void *a0, void *a1, void *a2, int (* module_start)(void *, void *, void *))
{
	u32 text_addr = ((u32)module_start) - 0x0BB8;
	
	/* Patch calls to sceKernelCheckExecFile */
	MAKE_CALL(text_addr+0x1554, sceKernelCheckExecFilePatched);
	MAKE_CALL(text_addr+0x15A4, sceKernelCheckExecFilePatched);
	MAKE_CALL(text_addr+0x4268, sceKernelCheckExecFilePatched);

	ClearCaches310();

	sceKernelCheckExecFile = (void *)(text_addr+0x3B9C);
	
	return module_start(a0, a1, a2);
}

int PatchLoadCore150(void *a0, void *a1, int (* module_start)(void *, void *))
{
	/* No Plain Module Check Patch */
	_sw(0x340D0001, 0x880152e0);
	ClearCaches150();

	return module_start(a0, a1);
}

int Main(void *a0, void *a1, void *a2, void *a3)
{	
	if (memcmp((void *)0x88c16CB2, "19196", 5) != 0)
	{
		/* Patch sceBootLfatOpen call */
		MAKE_CALL(0x88600094, sceBootLfatOpenPatched);

		/* Patch DecryptPSP to enable plain text config */
		// sw $a2, 0($sp) -> sw $a1, 0($sp) 
		// addiu v1, zero, $ffff -> addi	$v1, $a1, 0x0000
		// return -1 -> return a1 (size)
		_sw(0xafa50000, 0x88604e4c);	
		_sw(0x20a30000, 0x88604e50);

		/* patch point : removeByDebugSecion */
		/* Dummy a function to make it return 1 */
		_sw(0x03e00008, 0x88600f34);
		_sw(0x24020001, 0x88600f38);

		// patch init error
		_sw(0, 0x8860008c);

		/* Patch the call to LoadCore module_start */
		// 886049c8: jr $t3 -> mov $a3, $t3 /* a3 = LoadCore module_start */
		// 886049cc: mov $sp, $s5
		// 886049d0: nop -> jal PatchLoadCore310
		// 886049d4: j  0x88c045f4 (return 0) -> nop
		_sw(0x01603821, 0x886049c8);
		MAKE_CALL(0x886049d0, PatchLoadCore310);
		_sw(0, 0x886049d4);

		/* Patch check hash errors */
		_sw(0, 0x88603f34);
		_sw(0, 0x88603f8c);

		/*if (_lw(0x88f80004) == 0x27BDFFE0)
		{
			Real_Reboot = (void *)0x88f80000;			
		}*/

		/* Patch nand decryption */
		/* sw $a0, 0x8861c020 -> sw $zero, 0x8861c020 */
		_sw(0xac60c020, 0x88610148);

		/* key generation */
		buld_key_param(key_data_310);

		ClearCaches310();
	}

	else
	{
		/* Patch the call to LoadCore module_start */
		// 88c00fec: mov $v0, $s2 -> mov $a2, $s2 (module_start)
		// 88c00ff0: mov $a0, $s5
		// 88c00ff4: mov $a1, $sp
		// 88c00ff8: jr  $v0 -> jal PatchLoadCore150
		// 88c00ffc: mov $sp, $s6
		_sw(0x02403021, 0x88c00fec);
		MAKE_CALL(0x88c00ff8, PatchLoadCore150);
		Real_Reboot = (void *)0x88c00000;

		ClearCaches150();
	}	

	return Real_Reboot(a0, a1, a2, a3);	
}