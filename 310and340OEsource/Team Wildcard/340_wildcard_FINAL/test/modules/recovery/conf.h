/*------------------------------------------------------
	Project:		PSP-CFW*
	Module:			CFW_Config
	Maintainer:		
------------------------------------------------------*/

#ifndef _CFW_CONF_
#define _CFW_CONF_

#include "type.h"

#define CONFIG_MAGIC 0x47434553

enum FAKE_REGION
{
	FAKE_REGION_DISABLED = 0,
	FAKE_REGION_JAPAN = 1,
	FAKE_REGION_AMERICA = 2,
	FAKE_REGION_EUROPE = 3,
	FAKE_REGION_KOREA = 4,		/* do not use, may cause brick on restore default settings */
	FAKE_REGION_UNK = 5, 
	FAKE_REGION_UNK2 = 6,
	FAKE_REGION_AUSTRALIA = 7,
	FAKE_REGION_HONGKONG = 8,	/* do not use, may cause brick on restore default settings */
	FAKE_REGION_TAIWAN = 9,		/* do not use, may cause brick on restore default settings */
	FAKE_REGION_RUSSIA = 10,
	FAKE_REGION_CHINA = 11		/* do not use, may cause brick on restore default settings */
};

enum KERNEL_TYPE
{
	KERNEL_150 = false,
	KERNEL_3XX = true
};

//typedef struct _cfw_config
//{
//	u32 magic;						/* */
//	bool hidecorrupt;
//	bool skiplogo;
//	bool umdactivatedplaincheck;
//	bool gamekernel;				/* pass KERNEL_150 or KERNEL_3XX */
//	bool executebootbin;
//	bool startupprog;
//	bool usenoumd;
//	bool useisofsonumdinserted;
//	u16 vshcpuspeed;			/* default 222 */
//	u16	gamecpuspeed;			/* default 222 */
//	u16	popcpuspeed;			/* default 333 */
//	u8 fakeregion;
//	bool freeumdregion;
//} __attribute__((packed)) cfwConfig, *cfwConfig_p;

typedef struct _cfw_config
{
	int magic; /* 0x47434553 */
	int hidecorrupt;
	int	skiplogo;
	int umdactivatedplaincheck;
	int gamekernel150;
	int executebootbin;
	int startupprog;
	int usenoumd;
	int useisofsonumdinserted;
	int	vshcpuspeed;
	int	vshbusspeed;
	int	umdisocpuspeed;
	int	umdisobusspeed;
	int fakeregion;
	int freeumdregion;
} cfwConfig, *cfwConfig_p;

/*	get the cfw cofig info
	@param config - The pointer to get the cfwConfig struct.
	@returns true on succuss, or false on error*/
extern bool cfwGetConfig(cfwConfig_p config);

/*	set the cfw cofig info
	@param config - The pointer of the cfwConfig struct to set.
	@returns true on succuss, or false on error*/
extern bool cfwSetConfig(cfwConfig_p config);

#endif /*_CFW_CONF_*/
