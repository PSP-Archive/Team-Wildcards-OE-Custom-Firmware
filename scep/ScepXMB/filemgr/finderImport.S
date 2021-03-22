.set noreorder

#include "pspimport.s"

IMPORT_START	"scepGraphicLibrary",0x00010000
IMPORT_FUNC		"scepGraphicLibrary",0x45919AA5,initFont
IMPORT_FUNC		"scepGraphicLibrary",0xF13F5779,freeFont
IMPORT_FUNC		"scepGraphicLibrary",0x6FF08996,getBorder
IMPORT_FUNC		"scepGraphicLibrary",0x324CC2A3,drawLine
IMPORT_FUNC		"scepGraphicLibrary",0x08B1FEDE,drawCRect
IMPORT_FUNC		"scepGraphicLibrary",0x01ADD172,drawMCRect
IMPORT_FUNC		"scepGraphicLibrary",0x80D358AE,clearRect
IMPORT_FUNC		"scepGraphicLibrary",0x940548F7,clearCRect
IMPORT_FUNC		"scepGraphicLibrary",0x7EE402BF,drawBText
IMPORT_FUNC		"scepGraphicLibrary",0xFBDFAE3B,drawScroll
IMPORT_FUNC		"scepGraphicLibrary",0x77F52844,drawLScroll

IMPORT_START	"scepConfigLibrary",0x00010000
IMPORT_FUNC		"scepConfigLibrary",0x0750946A,getConfig
IMPORT_FUNC		"scepConfigLibrary",0x9DCCB813,saveConfig

IMPORT_START	"scepUiService",0x00010000
IMPORT_FUNC		"scepUiService",0x4FC97619,drawMenu
IMPORT_FUNC		"scepUiService",0x3C20CF8A,uiOsk
