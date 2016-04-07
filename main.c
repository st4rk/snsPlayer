/*
 * snsPlayer - NES/SNES Music Player
 * Written by St4rk
 */
 
#include "nsf.h"

/* 
 * This function is used to verify the file extension 
 */
unsigned char verifyFileExt(char *fileName) {
	unsigned int strSize = strlen(fileName);	
	char ext[5];

	/* copy file extension to ext */
	strcpy(ext, (fileName + (strSize - 4)));

	/* verify if it's spc or nsf" */
	if (strcmp(ext, ".nsf") == 0)
		return 0x0;

	if (strcmp(ext, ".spc") == 0)
		return 0x1;

	return 0x2;
}

int main(int argc, char **argv) {
	/* Verify there are three arguments */
	if (argc != 3) {
		printf("format *.nsf* or *.spc* *tracking num*\n");
		exit(0);
	}
	
	/* Verify file extension */
	switch (verifyFileExt(argv[1])) {
		case 0x0:
			nsf_play(argv[1], atoi(argv[2]));
		break;

		case 0x1:
			printf("Not implemented :^/ \n");
		break;

		case 0x2:
			printf("Invalid file, it should be an NSF or SPC\n");
		break;
	}

	return 0;
}