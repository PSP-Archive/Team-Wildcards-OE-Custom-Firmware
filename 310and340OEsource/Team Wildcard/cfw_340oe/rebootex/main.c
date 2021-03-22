/*
 * 
 * 3.40OE rebootex.bin
 *
 * This file is embedded in SystemControl, SystemControl150, & Reboot150.
 * The purpose of this file is to replace the real reboot.bin to do some
 * patches which then lets the real reboot.bin boot 3.40 off the /kn folder
 * 
 *	Version		Date		Person		Summary
 * ----------------------------------------------------------------
 * 	0.01		26/07/07	              	Currently based off 3.10OE
 *							Just updated 3.40 patches
 *							(HASNT BEEN TESTED!!)
 *	0.02
 *
 *
 */

#include <pspsdk.h>

int Main(void *, void *, void *, void *);

int Reboot_Entry(void *a0, void *a1, void *a2, void *a3)
{
	return Main(a0, a1, a2, a3);
}

#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000
#define MIPS_JAL(IMM) (0xC000000 + (u32)(IMM)&0x3FFFFFF)
#define MIPS_ADDI(RT,RS,IMM)    (0x24000000|(RS<<21)|(RT<<16)|((u32)(IMM)&0xffff))
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a); 

//#define CHANGE_FUNC(a, f) _sw(J_OPCODE | (((u32)(f) & 0x3FFFFFFF) >> 2), a); _sw(0, a+4);

int (* Real_Reboot)(void *a0, void *a1, void *a2, void *a3) = (void *)0x88600000;
int (* sceBootLfatOpen)(char *file) = (void *)0x88607EAC;
int (* DcacheClear340)(void) = (void *)0x886027C0;
int (* IcacheClear340)(void) = (void *)0x8860218C;
int (* DcacheClear150)(void) = (void *)0x88c02c64;
int (* IcacheClear150)(void) = (void *)0x88c02c90;
int (* sceKernelCheckExecFile)(void *buf, int *check);

#define XD 0x95

static const u8 key_data_340[0x10] = 
{
	0x3B^XD, 0x9B^XD, 0x1A^XD, 0x56^XD, 0x21^XD, 0x80^XD, 0x14^XD, 0xED^XD,
	0x8E^XD, 0x8B^XD, 0x08^XD, 0x42^XD, 0xFA^XD, 0x2C^XD, 0xDC^XD, 0x3A^XD
};

static void buld_key_param(const u8 *key_real)
{
	int i;
	u8 *dst  = (u8 *)(0xbfc00200 - 0x35231452); // KEY param
	u8 *src  = (u8 *)(0x88611E80 - 0x35231452); // KEY seed
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

void ClearCaches340()
{
	DcacheClear340();
	IcacheClear340();
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

int PatchLoadCore340(void *a0, void *a1, void *a2, int (* module_start)(void *, void *, void *))
{
	u32 text_addr = ((u32)module_start) - 0x0BB8;
	
	// Patch within sceKernelLoadModuleBootLoadCore function
	// Replace call sceKernelCheckExecFile with our own
	MAKE_CALL(text_addr+0x15C8, sceKernelCheckExecFilePatched);
	MAKE_CALL(text_addr+0x1618, sceKernelCheckExecFilePatched);

	// Patch within sceKernelProbeExecutableObject function
	// Replace call sceKernelCheckExecFile with our own
	MAKE_CALL(text_addr+0x468C, sceKernelCheckExecFilePatched);

	ClearCaches340();

	sceKernelCheckExecFile = (void *)(text_addr+0x3FB4);
	
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
		// Patch within sceKernelBootLoadFileFlash function
		// Replace call to sceBootLfatOpen with our own ('/kd' to '/kn')
		MAKE_CALL(0x88600094, sceBootLfatOpenPatched);

		// Patch within sceKernelCheckPspConfig function
		// Patch ~PSP header check (enable non ~PSP config file)
		_sw(0xafa50000, 0x8860514C);	
		_sw(0x20a30000, 0x88605150);

		// patch KDebugForKernel_24C32559 to always return 1
		_sw(0x03e00008, 0x88600FA0);
		_sw(0x24020001, 0x88600FA4);

		// Patch within sceKernelBootLoadFileFlash function
		// Patch sceBootLfatfsMount fail errors 
		_sw(0, 0x8860008c);

		// patch within kactivate function
		// Patch call to LoadCore module_start
		_sw(MIPS_ADDI(7, 21, 0), 0x88604C2C);
		_sw(MIPS_JAL(PatchLoadCore340), 0x88604C30);
		_sw(MIPS_ADDI(29, 8, 0), 0x88604C34);

		// Patch hash check errors
		_sw(0, 0x88604084);
		_sw(0, 0x886040DC);

		// Patch nand decryption
		_sw(0xAC60C7E0, 0x88610742);

		// key generation
		buld_key_param(key_data_340);

		ClearCaches340();
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