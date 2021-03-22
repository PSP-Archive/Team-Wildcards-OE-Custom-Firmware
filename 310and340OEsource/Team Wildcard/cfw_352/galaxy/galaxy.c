// galaxy.prx from M33 3.51
//
// Not complete
//

SceUID SceNpUmdMount_thid = -1;

int _start()
{
	u32 *mod = sceKernelFindModuleByName("sceThreadManager");
	u32 text_addr = *(mod+27);

	// Patch within ThreadManForKernel Library Export Table

	// Replace sceKernelCreateThread export with our own
	// Use this to capture when np9660.prx creates its threads
	_sw(sceKernelCreateThread_fake, text_addr+0x16BC0);

	// Replace sceKernelStartThread export with our own
	_sw(sceKernelStartThread_fake, text_addr+0x16D54);

	clear_cache();

	return 0;
}

SceUID sceKernelCreateThread_fake(char *name, SceKernelThreadEntry entry, int initPri, int stackSize, int attr, SceKernelThreadOptParam *option)
{
	SceUID thid = sceKernelCreateThread(*name, entry, initPri, stackSize, attr, *option);

	// Check if np9660.prx is the one creating thread
	if (strncmp(*name, "SceNpUmdMount", 13) == 0)
	{
		SceNpUmdMount_thid = thid;
	}

	return(thid);
}

int sceKernelStartThread_fake()
{
	if (thid != SceNpUmdMount_thid)
	{
		return (sceKernelStartThread(thid, argSize, argp));
	}

	u32 *mod = sceKernelFindModuleByName("sceNp9660_driver");
	sceNp9660_text_addr = *(mod+27);

	// Patch within the open_disc_image function

	// Patch InitForKernel_48348387 function to return 0x80000000
	// Function returns address of iso image
	_sw(0x3C028000, sceNp9660_text_addr+0x1808);

	// Replace the decryption function with our own
	_sw(MIPS_JAL(sub_324), sceNp9660_text_addr+0x181C);

	// Patch functions called by the new np9660 ioread & iodevctl funcs
	_sw(MIPS_JAL(sub_244), sceNp9660_text_addr+0x1D68);
	_sw(MIPS_JAL(sub_244), sceNp9660_text_addr+0x2DAC);

	_sw(MIPS_J(sceIoClose_fake), sceNp9660_text_addr+0x4348);

	dword_1100 = sceNp9660_text_addr+0x1220; // _sceUmdCheckMedium
	dword_1104 = sceNp9660_text_addr+0x2290; // _Np9660FdMutexLock
	dword_1118 = sceNp9660_text_addr+0x22CC; // _Np9660FdMutexUnLock
	
	return (sceKernelStartThread(thid, argSize, argp));
}



