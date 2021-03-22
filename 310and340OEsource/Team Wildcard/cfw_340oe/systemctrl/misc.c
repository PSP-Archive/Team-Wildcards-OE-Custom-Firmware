sub_000047B0()
{
  if(var_7E14 == 0)
  {
    return loc_00005754(var_7DD0);
  }
  else
  {
    return CisofileGetDiscSize(var_7DD0);
  }
}

sub_000047D0()
{
  var_8B8C.0x4 = a1;
  var_8B8C.0 = a0;
  var_8B8C.0x8 = a2;
  var_8B8C.0xC = a3;

  return sceKernelExtendKernelStack(0x2000, sub_00004A38, 0);
}

sub_00004A38()
{
  if(var_7E10 == 0)
  {
    //00004ac4
    int i;
    for(i = 16; i; i--)
    {
      if(sceIoLseek32(var_7DD0, 0, PSP_SEEK_CUR) >= 0)
      {
        break; //->00004af8
      }

      sub_00004684();
    }

    //.00004af8
    if(var_7E10 == 0)
    {
      return 0x80010013;
    }
  }

  //.00004a60
  if(var_7E14 == 0)
  {
    return loc_0000570C(var_8B8C.0, var_8B8C.0x4, var_8B8C.0x8, var_8B8C.0xC);
  }
  else
  {
    return loc_000058F4(var_8B8C.0, var_8B8C.0x4, var_8B8C.0x8, var_8B8C.0xC);
  }
}

sub_00004BE4(unk_a0)
{
  if(var_7DBC == unk_a0)
  {
    return var_7DBC;
  }

  var_7DBC = unk_a0;

  return sub_000047D0(unk_a0, 1, var_7E18, 0);
}

00004CFC()
{
  return 0;
}

00004D04()
{
  return 0;
}
