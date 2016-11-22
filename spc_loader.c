#include "spc_loader.h"

void spc_loadFile(char *fileName, spc_info *info) {
	FILE *spc = NULL;
	unsigned int fSize = 0;
	unsigned char *data = NULL;

	spc = fopen(fileName, "rb");

	if (spc == NULL) {
		printf("spc file: %s not found\n", fileName);
	}

	fseek(spc, 0, SEEK_END);
	fSize = ftell(spc);
	rewind(spc);
	
	data = (unsigned char*) malloc (sizeof(unsigned char)  * fSize);

	fread(data, 1, fSize, spc);

	/* clear the DSP */
	memset(&s_dsp, 0, sizeof(dsp));


	/* copy information */
	memcpy(&info->song_title, (data + 0x2E), 32);
	memcpy(&info->game_title, (data + 0x4E), 32);
	memcpy(&info->dumper_name, (data + 0x6E), 16);
	memcpy(&info->comment, (data + 0x7E), 32);
	memcpy(&info->dump_date, (data + 0x9E), 4);
	memcpy(&info->unused, (data+0xA2), 0x7);
	memcpy(&info->sec_before_fout, (data + 0xA9), 3);
	memcpy(&info->num_fade_ms, (data + 0xAC), 5);
	memcpy(&info->sound_artist, (data + 0xB0), 32);
	memcpy(&info->default_channel_disable, (data + 0xD0), 1);
	memcpy(&info->emu_used_to_dump, (data + 0xD1), 1);
	memcpy(&info->reserved, (data + 0xD2), 45);

	printf("Song Title: %s\n", info->song_title);
	printf("Game Title: %s\n", info->game_title);
	printf("Artist: %s\n", info->sound_artist);
	printf("Dumper Name: %s\n", info->dumper_name);
	printf("Dump Date: %s\n", info->dump_date);

	/* setup SPC700 registers */
	info->core->PC = *(unsigned short*)(data + 0x25);
	info->core->A  = *(unsigned char*)(data + 0x27);
	info->core->X  = *(unsigned char*)(data + 0x28);
	info->core->Y  = *(unsigned char*)(data + 0x29);
	info->core->PSW = *(unsigned char*)(data + 0x2A);
	info->core->SP = *(unsigned char*)(data + 0x2B);

	/* copy RAM data */
	memcpy(spc_mem, (data + 0x100), 65536);

	/* setup the dsp to the beginning */
	for (int i = 0; i < 128; i++) {
		dsp_write(i, *(unsigned char*)(data + 0x10100 + i));
	}

	memcpy((spc_mem+0xFFC0), (data + 0x101C0), 64);
}