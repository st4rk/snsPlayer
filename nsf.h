/*
 * NSF - NES Sound Format
 * Written By St4rk
 * This module is used to load NSF files and play them
 * All extra mappers aren't implemented yet, only bank switching
 */


#ifndef _NSF_H_
#define _NSF_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 
 * Here is the list of extra mapper that can be used 
 */
#define EXTRA_VRC6    0b10000000
#define EXTRA_VRC7    0b01000000
#define EXTRA_FDS     0b00100000
#define EXTRA_MMC5    0b00010000
#define EXTRA_NAMCO   0b00001000
#define EXTRA_SUNSOFT 0b00000100 

/*
 * Sound Region
 */
#define NTSC_TUNE     0b00000000
#define PAL_TUNE      0b10000000
#define PAL_NTSC_TUNE 0b01000000

/*
 * This is the NSF Header
 */
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

/* 
 * This is the NSF File (Header + File Buffer)
 */
typedef struct nsf_file {
	unsigned char *buffer;
	unsigned int   buffer_size;
	nsf_header header;
} nsf_file;

/* Load an NSF File */
extern unsigned int nsf_loadFile(char *fileName);
/* NSF Main Loop */
extern void nsf_play(char *fileName, int musicNum);
/* Show informations of the NSF loaded */
extern void nsf_showInfo();
/* Free all memory allocated */
extern void nsf_freeMemory();
/* Initialize a NSF music as well as the 6502 */
extern void nsf_initTune(unsigned char *isBank, unsigned char *mem, unsigned char *x, unsigned char *a, int *pc);

#endif