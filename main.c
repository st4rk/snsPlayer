#include "nsf.h"
#include "6502.h"
#include <time.h>


int main(int argc, char **argv) {
	clock_t tick, delay;
	unsigned short i = 0;
	/* Open your file */
	if (nsf_loadFile(argv[1]) != 0xDEADDEAD) {	
		/* show file information */
		nsf_showInfo();
		/* reset 6502 */
		CPU_reset();

		open_audio();
		/* Initialize apu stuff(triangle, noise, square...) */
		for (i = 0x4000; i < 0x4013; i++)
			writeMemory(i, 0x00);
		/* Initialize the Noise, Triangle, and two Pulse channel */
		writeMemory(0x4015, 0x0F);
		/* Initialize the APU Framecounter to 4-step-mode */
		writeMemory(0x4017, 0x40);

		/* initialize tune informations */
		nsf_initTune(memory, &X, &A, &PC);

		A = atoi(argv[2]);

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
			//printf("Square1 sweep: %d\n", squareList[0].swp.enable);
			//printf("Square1 env cflag: %d\n", squareList[0].env.c_flag);
			/* Get System Tick */
			tick = SDL_GetTicks();

			/* Execute the cpu with 114 ticks */
			CPU_execute(114);

			/* Execute it @60 hz */
			if (delay < tick) {
				delay += (1000/60);

				/* macgyver used to see if the jsr happened */
				if (macgyver_var) {
					PC = 0x5054;
					macgyver_var = 0;
				}

				/* APU Frame Counter */
				switch (cnt) {
					case 0:
						square1_envelope();
						square2_envelope();
					break;

					case 1:
						square1_envelope();
						square2_envelope();
						square1_len_cnt();
						square2_len_cnt();
						square1_sweep();
						square2_sweep();
					break;

					case 2:
						square1_envelope();
						square2_envelope();
					break;

					case 3:
						square1_envelope();
						square2_envelope();
					break;

					default:

					break;
				}

				/* check the frame count */
				cnt > 4 ? cnt = 0 : cnt++;
			}
		}
		
		
		nsf_freeMemory();
	} else {

	}

	close_audio();


	return 0;
}