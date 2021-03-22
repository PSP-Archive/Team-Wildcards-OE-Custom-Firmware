.set noreorder

#include "pspimport.s"

IMPORT_START	"scepGraphicLibrary",0x00010000
IMPORT_FUNC		"scepGraphicLibrary",0x6FF08996,getBorder
IMPORT_FUNC		"scepGraphicLibrary",0x324CC2A3,drawLine
IMPORT_FUNC		"scepGraphicLibrary",0x08B1FEDE,drawCRect
IMPORT_FUNC		"scepGraphicLibrary",0x80D358AE,clearRect

IMPORT_START	"scepConfigLibrary",0x00010000
IMPORT_FUNC		"scepConfigLibrary",0x0750946A,getConfig
