#include "spc_loader.h"



void spc_LoaderFile(char *fileName) {
	FILE *spc = NULL;


	spc = fopen(fileName, "rb");

	if (spc == NULL) {
		printf("spc file: %s not found\n", fileName);
	}


	
}