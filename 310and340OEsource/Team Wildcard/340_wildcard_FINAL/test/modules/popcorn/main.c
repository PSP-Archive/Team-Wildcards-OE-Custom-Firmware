#include <pspsdk.h>
#include <systemctrl.h>

#include <string.h>

PSP_MODULE_INFO("daxPopcornManager", 0x1007, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000
#define SC_OPCODE	0x0000000C
#define JR_RA		0x03e00008

#define NOP	0x00000000

#define MAKE_JUMP(a, f) _sw(J_OPCODE | (((u32)(f) & 0x0ffffffc) >> 2), a);
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a);
#define MAKE_SYSCALL(a, n) _sw(SC_OPCODE | (n << 6), a);
#define JUMP_TARGET(x) (0x80000000 | ((x & 0x03FFFFFF) << 2))

#define REDIRECT_FUNCTION(a, f) _sw(J_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a);  _sw(NOP, a+4);
#define MAKE_DUMMY_FUNCTION0(a) _sw(0x03e00008, a); _sw(0x00001021, a+4);
#define MAKE_DUMMY_FUNCTION1(a) _sw(0x03e00008, a); _sw(0x24020001, a+4);

APRS_EVENT prev_handler = NULL; //0x8B8

int module_start(SceSize args, void *argp) __attribute__((alias("_start")));

int (*sceKernelDeflateDecompress)(u8 *dest, u32 destSize, const u8 *src, u32 unknown);
//int (*sceKernelSetCompiledSdkVersion)(u32 version);

//00000000
void ClearCaches()
{
  sceKernelDcacheWritebackAll();

  sceKernelIcacheClearAll();
}

//0000001c
void RebootVSHWithError(u32 error)
{
  struct SceKernelLoadExecVSHParam param;
  u32 vshmain_args[0x20/4];

  memset(&param, 0, sizeof(param));
  memset(vshmain_args, 0, sizeof(vshmain_args));

  vshmain_args[0/4] = 0x0400;
  vshmain_args[4/4] = 0x20;
  vshmain_args[0x14/4] = error;

  param.size = sizeof(param);
  param.args = 0x400;
  param.argp = vshmain_args;
  param.vshmain_args_size = 0x400;
  param.vshmain_args = vshmain_args;
  param.configfile = "/kd/pspbtcnf.txt";

  sctrlKernelExitVSH(&param);
}

//00000090
int scePopsMan_6768B22F_Patched(SceUID fd, void *data, SceSize size)
{
  SceUID fd2 = sceIoOpen(sceKernelInitFileName(), PSP_O_RDONLY, 0);

  u32 header[0x28 / 4];
  sceIoRead(fd2, header, 0x28);

  sceIoLseek(fd2, header[0x24 / 4] + 0x400, PSP_SEEK_SET);

  int iBytes = sceIoRead(fd2, data, size);

  sceIoClose(fd2);

  return iBytes;
}

//00000124
int scePopsMan_0090B2C8_Patched(u8 *dest, u32 destSize, const u8 *src, u32 unknown)
{
  int k1 = pspSdkSetK1(0);

  if(dest < 0)
  {
    RebootVSHWithError((u32)dest);

    pspSdkSetK1(0);

    return 0;
  }

  int ret = sceKernelDeflateDecompress(dest, destSize, src, unknown);

  pspSdkSetK1(k1);

  return ret;
}

//000001EC
int OpenEncryptedPatched(char *path, void *keys, int flags, int filemode, int offset)
{
  SceUID fd = sceIoOpen(path, flags, filemode);
  if(fd < 0)
  {
    return fd;
  }

  sceIoLseek(fd, offset, PSP_SEEK_SET);

  u32 sig;
  sceIoRead(fd, &sig, sizeof(sig));
  if(sig != 0x44475000) //DGP
  {
    sceIoLseek(fd, offset, PSP_SEEK_SET);

    return(fd);
  }

  //000002ac
  sceIoClose(fd);

  fd = sceIoOpen(path, flags | 0x40000000, filemode);

  int ret;
  if((ret = sceIoIoctl(fd, 0x4100002, &offset, sizeof(offset), NULL, 0)) < 0
      || (ret = sceIoIoctl(fd, 0x4100001, keys, 0x10, NULL, 0)) < 0)
  {
    sceIoClose(fd);

    return ret;
  }

  return fd;
}

//0000033C
int OnPspRelSectionEvent(char *name, u8 *buf)
{
  if(!strcmp(name, "pops"))
  {
    //000003a8
    u32 text_addr = (u32)(buf + 0xC0);

    _sw(0, text_addr + 0x139AC);

    u32 x = _lw((u32)(buf + 0x1CC));
    _sw(x, text_addr + 0x13850);

    ClearCaches();
  }

  if(prev_handler)
  {
    return prev_handler(name, buf);
  }

  return 0;
}

int FindSystemCalls()
{
  sceKernelDeflateDecompress = sctrlHENFindFunction("sceSystemMemoryManager", "UtilsForKernel", 0xE8DB3CE6);

//  sceKernelSetCompiledSdkVersion = sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0x7591C7DB);

  return(sceKernelDeflateDecompress != 0 && sceKernelSetCompiledSdkVersion != 0);
}

//000003e8
int _start(SceSize args, void *argp)
{
  SceUID fd;

  if(!FindSystemCalls())
  {
    return 1;
  }

  if((fd = sceIoOpen(sceKernelInitFileName(), PSP_O_RDONLY, 0)) < 0)
  {
    return 1;
  }

  u32 header[0x28 / 4];
  sceIoRead(fd, header, 0x28);

  sceIoLseek(fd, header[0x24 / 4] + 0x400, PSP_SEEK_SET);

  sceIoRead(fd, header, 0x28);

  if(header[0] == 0x44475000)
  {
    return 1;
  }

  SceModule *mod;
  if(!(mod = sceKernelFindModuleByName("scePops_Manager")))
  {
    return 0;
  }

  u32 text_addr = mod->text_size; //bug in PSP SDK SceModule struct?

  sceKernelSetCompiledSdkVersion(sceKernelDevkitVersion());

  prev_handler = sctrlHENSetOnApplyPspRelSectionEvent(OnPspRelSectionEvent);

  _sw(0x1021, text_addr + 0xF0);

  REDIRECT_FUNCTION(text_addr + 0xE34, OpenEncryptedPatched);

  REDIRECT_FUNCTION(text_addr + 0x1E8, scePopsMan_0090B2C8_Patched);

  _sw(NOP, text_addr + 0x504);
  MAKE_CALL(text_addr + 0x514, scePopsMan_6768B22F_Patched);

  ClearCaches();

  return 0;
}

int module_stop(void)
{
	return 0;
}
