/*
 * fileInfo.h
 *
 *  Created on: 11.09.2013
 *      Author: Smiths
 */

#ifndef FILEINFO_H_
#define FILEINFO_H_


/// список типов файлов
typedef enum
{
	UNKNOWN = 0,
	WAV,
	MP2,
	MP3,
	MP4,
	AAC,
	MOD
} fileTypes_t;


fileTypes_t get_filetype(char * filename);


#endif /* FILEINFO_H_ */
