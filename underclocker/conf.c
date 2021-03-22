#include "common.h"

#include "conf.h"

#define isvalidvarchar(c) (is_alnum(c) || c == '_' || c == '=')

int is_alnum(char ch)
{
	int c;

	c = toupper(ch);
	if((c >= 'A') && (c <= 'Z'))
	{
		return 1;
	}

	if((c >= '0') && (c <= '9'))
	{
		return 1;
	}

	return 0;
}
int is_aspace(int ch)
{
	if((ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r'))
	{
		return 1;
	}

	return 0;
}

int strcasecmp (const char *s1, const char *s2)
{
  int c1, c2;
  do
    {
      c1 = *((unsigned char *)(s1++));
      if (c1 >= 'A' && c1 <= 'Z')
        c1 = c1 + ('a' - 'A');
      c2 = *((unsigned char *)(s2++));
      if (c2 >= 'A' && c1 <= 'Z')
        c2 = c2 + ('a' - 'A');
      if (c1 != c2)
        {
          return (c1 - c2);
        }
    }
  while (c1 != 0);
  return 0;
}

static int read_line(SceUID fd, char *line, int max)
{
	int i, read;
	char ch;

	i = 0;

	if (max == 0)
		return 1;

	do
	{
		read = sceIoRead(fd, &ch, 1);
		
		if (read && ch != '\n' && ch != '\r')
			line[i++] = ch;		

	} while (ch != '\n' && read == 1 && i < max);

	line[i] = 0;

	return !read;
}

static int get_tokens(char tokens[][128], int maxtokens, char *line)
{
	int iline = 0;
	int itoken = 0;
	int jtoken = 0;
	int intoken = 0;
	int instring = 0;
	char ch;

	while (itoken < maxtokens)
	{
		ch = line[iline++];

		if (ch == 0)
		{
			if (instring)
				return 0; // Error: not terminated string

			if (intoken)			
				tokens[itoken++][jtoken] = 0;

			break;
		}

		if (!instring && (ch == '#' || ch == ';'))
		{
			if (intoken)			
				tokens[itoken++][jtoken] = 0;

			break;
		}

		if (is_aspace(ch) || ch == '=')
		{
			if (intoken)
			{
				if (!instring)
				{
					intoken = 0;
					tokens[itoken++][jtoken] = 0;
					jtoken = 0;
				}

				else
				{
					tokens[itoken][jtoken++] = ch;
				}
			}
		}

		else if (ch == '"')
		{
			if (intoken)
			{
				if ((!instring && jtoken != 0) || 
					(instring && isvalidvarchar(line[iline])))
				{
					// Error: Mixing string token with something else 
					return 0; 
				}

				tokens[itoken][jtoken++] = ch;				
				instring = !instring;
			}
			
			else
			{
				intoken = 1;
				instring = 1;
				tokens[itoken][jtoken++] = ch;
			}
		}

		else if (isvalidvarchar(ch))
		{
			if (!intoken)
				intoken = 1;

			tokens[itoken][jtoken++] = ch;
		}

		else
		{
			if (instring)
				tokens[itoken][jtoken++] = ch;
		}
	}

	return itoken;
}

static int get_integer(char *str)
{
	return strtol(str, NULL, 0);
}

static int get_boolean(char *str)
{
	if (strcasecmp(str, "false") == 0)
		return 0;

	if (strcasecmp(str, "true") == 0)
		return 1;	

	if (strcasecmp(str, "off") == 0)
		return 0;

	if (strcasecmp(str, "on") == 0)
		return 1;

	return get_integer(str);
}

static char *get_string(char *out, int max, char *in)
{
	char *p;
	int len;
	
	memset(out, 0, max);

	if (in[0] != '"')
		return NULL;

	p = strchr(in+1, '"');

	if (!p)
		return NULL;

	if (p-(in+1) > max)
		len = max;
	else
		len = p-(in+1);

	strncpy(out, in+1, len);	

	return out;
}

void read_config(const char *file, CONFIGFILE *config)
{
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0777);

	/* Default: all values to zero */
	memset(config, 0, sizeof(CONFIGFILE));

	if (fd > 0)
	{
		int eof = 0, ntokens;
		char line[128];
		char tokens[2][128];
		
		while (!eof)
		{
			eof = read_line(fd, line, 127);
			ntokens = get_tokens(tokens, 2, line);	

			if (ntokens == 2)
			{
 				if (strcasecmp(tokens[0], "triggerButton1") == 0)
 				{
					get_string(config->triggerButton1, 64, tokens[1]);
 					//config->triggerButton1 = get_integer(tokens[1]);
 				}
 				else if (strcasecmp(tokens[0], "triggerButton2") == 0)
 				{
					get_string(config->triggerButton2, 64, tokens[1]);
 					//config->triggerButton2 = get_integer(tokens[1]);
 				}
				else if (strcasecmp(tokens[0], "defaultCpuSpeed") == 0)
				{
					config->defaultCpuSpeed = get_integer(tokens[1]);
				}
				else if (strcasecmp(tokens[0], "defaultBusSpeed") == 0)
				{
					config->defaultBusSpeed = get_integer(tokens[1]);
				}
				else if (strcasecmp(tokens[0], "autoUnderclock") == 0)
				{
					config->autoUnderclock = get_boolean(tokens[1]);
				}
 				else if (strcasecmp(tokens[0], "autoUnderclock_timeout") == 0)
 				{
 					config->autoUnderclock_timeout = get_integer(tokens[1]);
 				}
			}
		}
		sceIoClose(fd);
	}
}
