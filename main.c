#include "nsf.h"
#include "6502.h"



int main(int argc, char **argv) {
	unsigned short rate;
	unsigned short i = 0;
	/* Open your file */
	if (nsf_loadFile("MegaMan2.nsf") != 0xDEADDEAD) {	
		/* show file information */
		nsf_showInfo();
		/* reset 6502 */
		CPU_reset();

		/* Initialize apu stuff(triangle, noise, square...) */
		for (i = 0x4000; i < 0x4013; i++)
			writeMemory(i, 0x00);
		/* Initialize the Noise, Triangle, and two Pulse channel */
		writeMemory(0x4015, 0x0F);
		/* Initialize the APU Framecounter to 4-step-mode */
		writeMemory(0x4017, 0x40);

		/* initialize tune informations */
		nsf_initTune(memory, &X, &A, &PC);

		while (1) {
			CPU_execute(0);
			if (macgyver_var)
				break;
		}

		while (1) {

			PC = 0x5054;
			macgyver_var = 0;
			while (1) {
				CPU_execute(0);

				if (macgyver_var)
					break;
			}
		}

		printf("NSF File Initialized with success !\n");

		rate = (1000000/nsf_getPlaySpeed());

		printf("Rate: %d\nhz", rate);

		nsf_freeMemory();
	} else {

	}
	return 0;
}