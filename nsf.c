/*
 * NSF - NES Sound Format
 * Written By St4rk
 * This module is used to load NSF files and play them
 * All extra mappers aren't implemented yet, only bank switching
 */

#include "nsf.h"
#include "6502.h"

/* List of NSF in folder */
nsf_file fileList;

/* Current track number */
unsigned int trackNum = 0;

/**
 * conio function, used to async keyboard input
 */
int kbhit(void) {

    struct timeval tv;

	fd_set read_fd;

	tv.tv_sec=0;

	tv.tv_usec=0;

	FD_ZERO(&read_fd);

	FD_SET(0,&read_fd);

	if(select(1, &read_fd, NULL, NULL, &tv) == -1)
		return 0;

	if(FD_ISSET(0,&read_fd))
		return 1;

	return 0;

}

/* 
 * This function load an NSF file into memory and load the NSF header from it
 */
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

/* 
 * This function free the memory allocated by buffer
 */
void nsf_freeMemory() {
	/* Verify if the buffer is not equal NULL */
	if (fileList.buffer != NULL) {
		/* if no, clear the memory */
		free(fileList.buffer);
		fileList.buffer      = NULL;
		fileList.buffer_size = 0;
	}
}

/* 
 * Display information of the sound to be play 
 */
void nsf_showInfo() {
	printf("-- NSF Information --\n");
	printf("Name:      %s\n", fileList.header.songName);
	printf("Artist:    %s\n", fileList.header.artistName);
	printf("Copyright: %s\n", fileList.header.copyright);
	printf("Musics:    %d\n", fileList.header.tSounds);
	printf("Track:     %d\n", trackNum);
}


/*
 * This function initialize the 6502, clear the memory, verify if bank switching
 * or any kind of mapper is used and finally initialize the music to be play
 */
void nsf_initTune(unsigned char *isBank, unsigned char *mem, unsigned char *x, unsigned char *a, int *pc) {
	int i = 0;

	/* Clear system memory */
	for (i = 0x0000; i < 0x07FF; i++)
		mem[i] = 0x00;

	for (i = 0x6000; i < 0x7FFF; i++)
		mem[i] = 0x00;


	/* Verify if is necessary bank switching */
	for (i = 0; i < 8; i++) {
		if (fileList.header.bankSwitchInitValues[i] != 0x00) {
			*isBank = 1;
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
			printf("No extra chip necessary 0x%X !\n",fileList.header.extraSoundChip );
		break;
	}

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
	if (fileList.buffer_size >= 0x8000)
		fileList.buffer_size = 0x7000;
	
	/* verify if it has bank switching, if yes, padding the rom and switch
	 * banks 
	 */
	if (*isBank) {
		memcpy(&mem[0x8000+(fileList.header.lAddrData & 0x0FFF)], &fileList.buffer[0x80], 0xFFFF);

		for (i = 0; i < 8; i++) {
			mem[0x5ff8 + i] = fileList.header.bankSwitchInitValues[i];
		}

	} else {
		memcpy(&mem[fileList.header.lAddrData], &fileList.buffer[0x80], fileList.buffer_size);
	}


	printf("Load Addr: 0x%X\n", fileList.header.lAddrData);
	printf("Init Addr: 0x%X\n", fileList.header.iAddrData);
	printf("Play Addr: 0x%X\n", fileList.header.pAddrData);


	*pc = 0x5050;

}

/* 
 * This is the NSF main loop 
 */
void nsf_play(char *fileName, int musicNum) {
	/* Used to run the loop with 60Hz */
	unsigned int delay = 0;
	unsigned int tick  = 0;
	/* Used by for */
	unsigned int i     = 0;

	/* store the first track to be sign */
	trackNum = musicNum;

initAgain:

#ifdef WIN32
	system("cls");
#endif
#ifdef __LINUX__
	system("clear");
#endif

	/* Open your file */
	if (nsf_loadFile(fileName) != 0xDEADDEAD) {	
		
		/* show file information */
		nsf_showInfo();
		
		/* reset 6502 */
		CPU_reset();
		
		/* Initialize SDL_Mix */
		open_audio();

		/* Initialize apu stuff(triangle, noise, square...) */
		for (i = 0x4000; i < 0x4013; i++)
			writeMemory(i, 0x00);
		
		/* Initialize the Noise, Triangle, and two Pulse channel */
		writeMemory(0x4015, 0x0F);
		
		/* Initialize the APU Framecounter to 4-step-mode */
		writeMemory(0x4017, 0x40);

		/* initialize tune informations */
		nsf_initTune(&isBank, memory, &X, &A, &PC);

		A = trackNum;

		while (1) {
			CPU_execute(114);
			if (macgyver_var)
				break;
		}

		/* Restart the process */
		PC = 0x5054;
		macgyver_var = 0;

		printf("NSF File Initialized with success !\n");

		/* APU Frame Counter Var */
		int cnt = 0;
		while (1) {

			/* 
			 * verify if there is key into keyboard buffer;
			 */
			if (kbhit()) {
				unsigned char c = getchar();

	 			switch (c) {
	 				case 'n':
	 					if (trackNum < fileList.header.tSounds) {
	 						trackNum++;
	 						goto initAgain;
	 					}
	 				break;

	 				case 'b':
	 					if (trackNum > 0) {
	 						trackNum--;
	 						goto initAgain;
	 					}
	 				break;
	 			}

			}

			/* Execute the cpu with 114 ticks */
			CPU_execute(114);

			/* Execute it @60 hz */
			if (delay < SDL_GetTicks()) {
				delay = SDL_GetTicks() + 1000/60;


				/* macgyver used to see if the jsr happened */
				if (macgyver_var) {
					PC = 0x5054;
					macgyver_var = 0;
				}

				/* APU Frame Counter */
				switch (cnt) {
					case 0:
					case 2:
						square_envelope(SQUARE_WAVE_UNIT_1);
						square_envelope(SQUARE_WAVE_UNIT_2);
						noise_envelope();
						triangle_linear_cnt();
					break;

					case 1:
					case 3:
						square_envelope(SQUARE_WAVE_UNIT_1);
						square_envelope(SQUARE_WAVE_UNIT_2);
						square_len_cnt(SQUARE_WAVE_UNIT_1);
						square_len_cnt(SQUARE_WAVE_UNIT_2);
						square_sweep(SQUARE_WAVE_UNIT_1);
						square_sweep(SQUARE_WAVE_UNIT_2);
						noise_len_cnt();
						noise_envelope();
						triangle_len_cnt();
						triangle_linear_cnt();
					break;
				}

				/* check the frame count */
				cnt > 4 ? cnt = 0 : cnt++;
			} 
		}
		
		/* Clear memory */	
		nsf_freeMemory();
	}

	/* Stop the SDL_mixer */
	close_audio();
}