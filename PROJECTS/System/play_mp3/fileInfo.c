/*
 * fileInfo.c
 *
 *  Created on: 11.09.2013
 *      Author: Smiths
 */


//*-----------------------------------------------------------------------------------------------
//*			Внешние модули
//*-----------------------------------------------------------------------------------------------
#include "includes.h"


//*-----------------------------------------------------------------------------------------------
//*-----------------------------------------------------------------------------------------------
fileTypes_t get_filetype(char * filename)
{
	char *extension;

	extension = strrchr(filename, '.') + 1;

	if(strncasecmp(extension, "MP2", 3) == 0)
	{
		return MP2;
	}
	else if (strncasecmp(extension, "MP3", 3) == 0)
	{
		return MP3;
	}
	else if (strncasecmp(extension, "MP4", 3) == 0 ||
	         strncasecmp(extension, "M4A", 3) == 0)
	{
		return MP4;
	}
	else if (strncasecmp(extension, "AAC", 3) == 0)
	{
		return AAC;
	}
	else if (strncasecmp(extension, "WAV", 3) == 0 ||
	         strncasecmp(extension, "RAW", 3) == 0)
	{
		return WAV;
	}
	else if (strncasecmp(extension, "MOD", 3) == 0)
	{
		return MOD;
	}
	else
	{
		return UNKNOWN;
	}
}

