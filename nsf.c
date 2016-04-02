#include "nsf.h"

/* List of NSF in folder */
nsf_file fileList;

/* Load an NSF to fileList */
unsigned int nsf_loadFile(char *fileName) {
	FILE *arq = NULL;
	unsigned int fileSize = 0;

	/* open file */
	arq = fopen(fileName, "rb");

	/* check if the file is in directory */
	if (arq == NULL) {
		printf("File %s not found\n", fileName);
		return 0xDEADDEAD;
	}

	/* Get file size */
	fseek(arq, 0L, SEEK_END);
	fileSize = ftell(arq);
	rewind(arq);

	/* Check if the file is big than 0 */
	if (fileSize == 0) {
		printf("File %s size is 0", fileName);
		fclose(arq);
		return 0xDEADDEAD;
	}

	/* Alloc a buffer with the size of file */
	fileList.buffer_size = fileSize;
	fileList.buffer      = (unsigned char *) malloc(sizeof(unsigned char) * fileSize);

	/* Read our file to the buffer */ 
	fread(fileList.buffer, 1, fileList.buffer_size, arq);

	/* Read File information */
	memcpy(&fileList.header, fileList.buffer, (sizeof(nsf_header)));
	/* Free the file */
	fclose (arq);

	return 1;
}

/* should be always called in the end of program */
void nsf_freeMemory() {
	/* Verify if the buffer is not equal NULL */
	if (fileList.buffer != NULL) {
		/* if no, clear the memory */
		free(fileList.buffer);
		fileList.buffer      = NULL;
		fileList.buffer_size = 0;
	}
}

/* Display information of the sound to be play */
void nsf_showInfo() {
	printf("-- NSF Information --\n");
	printf("Name:      %s\n", fileList.header.songName);
	printf("Artist:    %s\n", fileList.header.artistName);
	printf("Copyright: %s\n", fileList.header.copyright);
	printf("Musics: %d\n", fileList.header.tSounds);
}


void nsf_initTune(unsigned char *mem, unsigned char *x, unsigned char *a, int *pc) {
	int i = 0;
	unsigned char isBankSwitch = 0;
	/* Verify if is necessary bank switching */
	for (i = 0; i < 8; i++) {
		if (fileList.header.bankSwitchInitValues[i] != 0x00) {
			isBankSwitch = 1;
		}
	}

	/* verify the tune region, X hold the region*/
	switch (fileList.header.PAL_NTSC) {
		case NTSC_TUNE:
			printf("It's a NTSC tune\n");
			*x = 0;
		break;

		case PAL_TUNE:
			printf("It's a PAL tune\n");
			*x = 1;
		break;

		case PAL_NTSC_TUNE:
			printf("It's a dual PAL/NTSC tune\n");
			*x = 0;
		break;

		default:
			printf("Something is wrong with your tune file\n");
		break;
	}

	/* Verify if an extra sound chip is used */
	switch (fileList.header.extraSoundChip) {
		case EXTRA_VRC6:
			printf("VRC6 audio unsupported\n");
		break;

		case EXTRA_VRC7:
			printf("VRC7 audio unsupported\n");
		break;

		case EXTRA_FDS:
			printf("FDS audio unsupported\n");
		break;

		case EXTRA_MMC5:
			printf("MMC5 audio unsupported\n");
		break;

		case EXTRA_NAMCO:
			printf("NAMCO audio unsupported\n");
		break;

		case EXTRA_SUNSOFT:
			printf("SUNSOFT audio unsupported\n");
		break;

		default:
			printf("No extra chip necessary !\n");
		break;
	}

	/* Clear system memory */
	for (i = 0x0000; i < 0x07FF; i++)
		mem[i] = 0x00;

	for (i = 0x6000; i < 0x7FFF; i++)
		mem[i] = 0x00;


	/* MacGyver */
	mem[0x5050] = 0x20; // JSR
	mem[0x5051] = (fileList.header.iAddrData & 0x00FF);
	mem[0x5052] = ((fileList.header.iAddrData & 0xFF00) >> 8);
	mem[0x5053] = 0xFF;

	mem[0x5054] = 0x20; // JSR
	mem[0x5055] = (fileList.header.pAddrData & 0x00FF);
	mem[0x5056] = ((fileList.header.pAddrData & 0xFF00) >> 8);
	mem[0x5057] = 0xFF;


	/* Load file into 6502 memory */
	fileList.buffer_size -= 0x80;
	for (i = 0x0; i < fileList.buffer_size; i++) {
		mem[i + fileList.header.lAddrData] = fileList.buffer[i + 0x80];
	}

	/* A hold the sound number */
	*a = 69;
	/* PC is now the init routine */
	printf("init addr: 0x%X\n",fileList.header.pAddrData);
	*pc = 0x5050;

}


unsigned short nsf_getPlayAddr() { return fileList.header.pAddrData; }
unsigned short nsf_getPlaySpeed() { return  fileList.header.playSpeed_NTSC; }