#ifndef _NSF_H_
#define _NSF_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXTRA_VRC6    0b10000000
#define EXTRA_VRC7    0b01000000
#define EXTRA_FDS     0b00100000
#define EXTRA_MMC5    0b00010000
#define EXTRA_NAMCO   0b00001000
#define EXTRA_SUNSOFT 0b00000100 

#define NTSC_TUNE     0b00000000
#define PAL_TUNE      0b10000000
#define PAL_NTSC_TUNE 0b01000000

typedef struct nsf_header {
	char nsf_magic[5];
	unsigned char verNum;
	unsigned char tSounds;
	unsigned char sSounds;

	unsigned short lAddrData;
	unsigned short iAddrData;
	unsigned short pAddrData;

	char songName[32];
	char artistName[32];
	char copyright[32];

	unsigned short playSpeed_NTSC;
	unsigned char  bankSwitchInitValues[8];
	unsigned short playSpeed_PAL;

	unsigned char  PAL_NTSC;
	unsigned char  extraSoundChip;
	unsigned char  expansion[4];
} nsf_header;


typedef struct nsf_file {
	unsigned char *buffer;
	unsigned int   buffer_size;
	nsf_header header;
} nsf_file;


extern unsigned int nsf_loadFile(char *fileName);

extern unsigned short nsf_getPlayAddr();
extern unsigned short nsf_getPlaySpeed();

extern void nsf_showInfo();
extern void nsf_freeMemory();
extern void nsf_initTune(unsigned char *mem, unsigned char *x, unsigned char *a, int *pc);

#endif