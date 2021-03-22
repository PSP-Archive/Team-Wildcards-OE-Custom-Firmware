#ifndef __CONF_H__
#define __CONF_H__

typedef struct
{
	char triggerButton1[100];
	char triggerButton2[100];
	unsigned int defaultCpuSpeed;
	unsigned int defaultBusSpeed;
	int autoUnderclock;
	unsigned int autoUnderclock_timeout;

} CONFIGFILE;

void read_config(const char *file, CONFIGFILE *config);

#endif
