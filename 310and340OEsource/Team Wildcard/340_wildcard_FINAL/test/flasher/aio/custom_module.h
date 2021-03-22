#ifndef __CUSTOM_MODULE_H__
#define __CUSTOM_MODULE_H__

typedef struct
{
  char  *source;
  char  *destination;
  int   size;
  u8    *data;
} CustomModule;

#include "custom_module.in"

#endif
