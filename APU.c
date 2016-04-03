#include "APU.h"

/* The total of samples is determined by the sample rate div frequency */
#define SQUARE1_SAMPLE (int) (44100/squareList[0].out_freq)
#define SQUARE2_SAMPLE (int) (44100/squareList[1].out_freq)

/* NES APU has two square wave channel */
square_wave squareList[2];
/* NES APU flags */
apu_status  apu;
/* Sample count used by the Square waves */
int square1_sample_cnt = 0;
int square2_sample_cnt = 0;


void fill_audio(void *data, Uint8 *stream, int len) {
	short *buff;
	int i;

	buff = (short*)stream;

	len /=2; /* short  */
	for (i = 0; i < len; i+= 2) {
		buff[i]   = square_mix();
		buff[i+1] = buff[i]; 
	}
}



void open_audio() {
	SDL_AudioSpec as;
	SDL_Init(SDL_INIT_AUDIO);
	as.freq     = 44100;
	as.format   = AUDIO_S16SYS;
	as.channels = 2;
	as.samples  = 4096;
	as.callback = fill_audio;

	SDL_OpenAudio(&as, NULL);
	SDL_PauseAudio(0);
}

void close_audio() {
	SDL_CloseAudio();
	SDL_Quit();
}



/* Square Wave Channel 1 */
short square1_sample() {
	if (squareList[0].out_freq > 0) {
		square1_sample_cnt++;

		if (square1_sample_cnt >= SQUARE1_SAMPLE)
			square1_sample_cnt = 0;

		switch (squareList[0].duty) {
			case 0:
				if (square1_sample_cnt < (int)(SQUARE1_SAMPLE * 0.125))
					return -(100 * squareList[0].env.volume);
			
				return  (100 * squareList[0].env.volume);
			break;

			case 1:
				if (square1_sample_cnt < (int)(SQUARE1_SAMPLE * 0.25))
						return -(100 * squareList[0].env.volume);
				
					return (100 * squareList[0].env.volume);
			break;

			case 2:
				if (square1_sample_cnt < (int)(SQUARE1_SAMPLE * 0.5))
						return -(100 * squareList[0].env.volume);
				
					return (100 * squareList[0].env.volume);
			break;

			case 3:
				if (square1_sample_cnt < (int)(SQUARE1_SAMPLE * 0.25))
						return (100 * squareList[0].env.volume);
				
					return -(100 * squareList[0].env.volume);
			break;

			default:
				printf("aeho\n");
			break;
		}

	}
	return 0;
}

/* Control the wave amplitude */
void square1_envelope() {

	/* verify if declay flag is not set, if it's not, should decay the volume */
	if (!(squareList[0].env.c_flag)) {
		/* The envelope starts with volume 15, and decrements every time the unit is clocked */
		if (squareList[0].env.volume > 0) 
			squareList[0].env.volume--;
		else {
			/* Check if the envelope loop flag is enable, if it's, should to load
			0xF(15) to volume and count down again */
			if (squareList[0].env.loop_flag == 1) 
				squareList[0].env.volume = 0xF;
		}
	}
}


void square1_freq_output() {
	/* The output frequency of the generator can be determined by the timer(period)
	if the timer has a value < 8, the channel is silenced */

	if (squareList[0].timer > 8) {
		squareList[0].out_freq = (unsigned int)(NTSC_CPU_CLOCK / (16 * (squareList[0].timer + 1))); 
	} else {
		squareList[0].out_freq = 0;
	}

}

/* Control wave period */
void square1_sweep() {
	unsigned short newPeriod = 0x0;
	if (squareList[0].swp.enable) {
		/* if the divider counter is not equal 0, decrement */
		if (squareList[0].swp.divider_cnt > 0)
			squareList[0].swp.divider_cnt--;
		else {
			/* it's equal a 0, the counter should be set with P */
			squareList[0].swp.divider_cnt = squareList[0].swp.period;
			/* the pulse's period is adjusted */

			/* Verify if timer >= 8, otherwise the channel should be silenced */
			if (squareList[0].timer >= 8) {
				/* Verify if negated flag is enable, if yes, you should decrement the 
				shift value from timer, on channel 2, you should increment + 1 to the final result*/

				if (squareList[0].swp.neg_flag) /* a negative sweep on pulse channel 1 will subtract the shifted period value minus 1. */
					newPeriod = (squareList[0].timer - (squareList[0].timer >> squareList[0].swp.shift_cnt) - 1);
				else
					newPeriod = (squareList[0].timer + (squareList[0].timer >> squareList[0].swp.shift_cnt));
			

				/* if the target period (current period(timer) + shift result) is greater than 0x7FF
				the channel should be silenced */
				if (newPeriod > 0x7FF) {
					squareList[0].env.volume = 0;
				} else {
					if ((squareList[0].swp.enable) && (squareList[0].swp.shift_cnt != 0)) {
						/* Store new period and update the frequency */
						squareList[0].timer = newPeriod;
						square1_freq_output(); 
					}
				}

			} else {
				squareList[0].env.volume = 0;
			}
		}
	}
}


void square1_len_cnt() {
	/* If enable bit is set on status, force the length counter do 0 */
	if (apu.pulse_channel_1 == 0x0) {
		squareList[0].len_cnt = 0;
		/* When it reaches to 0, the sound of channel should be silenced */
		squareList[0].env.volume = 0;
	} else {
		/* Check if halt flag is not set and check if the len_cnt is not already 0 */
		if (squareList[0].env.loop_flag == 0) {
			if (squareList[0].len_cnt > 0) 
				squareList[0].len_cnt--;
		}
	}
}


/* Square Wave Channel 1 */
short square2_sample() {
	if (squareList[1].out_freq > 0) {
		square2_sample_cnt++;

		if (square2_sample_cnt >= SQUARE2_SAMPLE)
			square2_sample_cnt = 0;

		switch (squareList[1].duty) {
			case 0:
				if (square2_sample_cnt < (int)(SQUARE2_SAMPLE * 0.125))
					return -(100 * squareList[1].env.volume);
			
				return  (100 * squareList[1].env.volume);
			break;

			case 1:
			if (square2_sample_cnt < (int)(SQUARE2_SAMPLE * 0.25))
					return -(100 * squareList[1].env.volume);
			
				return (100 * squareList[1].env.volume);
			break;

			case 2:
			if (square2_sample_cnt < (int)(SQUARE2_SAMPLE * 0.5))
					return -(100 * squareList[1].env.volume);
			
				return (100 * squareList[1].env.volume);
			break;

			case 3:
			if (square2_sample_cnt < (int)(SQUARE2_SAMPLE * 0.25))
					return (100 * squareList[1].env.volume);
			
				return -(100 * squareList[1].env.volume);
			break;

			default:

			break;
		}

	}
	return 0;
}

/* Control the wave amplitude */
void square2_envelope() {

	/* verify if declay flag is not set, if it's not, should decay the volume */
	if (!(squareList[1].env.c_flag)) {
		/* The envelope starts with volume 15, and decrements every time the unit is clocked */
		if (squareList[1].env.volume > 0) 
			squareList[1].env.volume--;
		else {
			/* Check if the envelope loop flag is enable, if it's, should to load
			0xF(15) to volume and count down again */
			if (squareList[1].env.loop_flag == 1) 
				squareList[1].env.volume = 0xF;
		}
	}
}


void square2_freq_output() {
	/* The output frequency of the generator can be determined by the timer(period)
	if the timer has a value < 8, the channel is silenced */

	if (squareList[1].timer > 8) {
		squareList[1].out_freq = (NTSC_CPU_CLOCK / (16 * (squareList[1].timer + 1))); 
	} else {
		squareList[1].out_freq = 0;
	}

}

/* Control wave period */
void square2_sweep() {
	unsigned short newPeriod = 0x0;
	if (squareList[1].swp.enable) {
		/* if the divider counter is not equal 0, decrement */
		if (squareList[1].swp.divider_cnt > 0)
			squareList[1].swp.divider_cnt--;
		else {
			/* it's equal a 0, the counter should be set with P */
			squareList[1].swp.divider_cnt = squareList[0].swp.period;
			/* the pulse's period is adjusted */

			/* Verify if timer >= 8, otherwise the channel should be silenced */
			if (squareList[1].timer >= 8) {
				/* Verify if negated flag is enable, if yes, you should decrement the 
				shift value from timer, on channel 2, you should increment + 1 to the final result*/

				if (squareList[1].swp.neg_flag) /* a negative sweep on pulse channel 1 will subtract the shifted period value minus 1. */
					newPeriod = (squareList[1].timer - (squareList[1].timer >> squareList[1].swp.shift_cnt) - 1);
				else
					newPeriod = (squareList[1].timer + (squareList[1].timer >> squareList[1].swp.shift_cnt));
			

				/* if the target period (current period(timer) + shift result) is greater than 0x7FF
				the channel should be silenced */
				if (newPeriod > 0x7FF) {
					squareList[1].env.volume = 0;
				} else {
					if ((squareList[1].swp.enable) && (squareList[1].swp.shift_cnt != 0)) {
						/* Store new period and update the frequency */
						squareList[1].timer = newPeriod;
						square2_freq_output(); 
					}
				}

			} else {
				squareList[1].env.volume = 0;
			}
		}
	}
}

void square2_len_cnt() {
	/* If enable bit is set on status, force the length counter do 0 */
	if (apu.pulse_channel_2 == 0x0) {
		squareList[1].len_cnt = 0;
		/* When it reaches to 0, the sound of channel should be silenced */
		squareList[1].env.volume = 0;
	} else {
		/* Check if halt flag is not set and check if the len_cnt is not already 0 */
		if (squareList[1].env.loop_flag == 0) {
			if (squareList[1].len_cnt > 0) 
				squareList[1].len_cnt--;
		}
	}
}


/* APU Lenght Count Table */
unsigned char square_getLenghtCnt(unsigned char len) {
	/* Verify bit 3 */
	if (!(len & 0x8)) {
		switch (((len & 0x70) >> 0x4)) {
			case 0x0:
				if (len & 0x80)
					return 0x06;
				else
					return 0x05;
			break;

			case 0x1:
				if (len & 0x80)
					return 0xC;
				else
					return 0xA;
			break;

			case 0x2:
				if (len & 0x80)
					return 0x18;
				else
					return 0x14;
			break;

			case 0x3:
				if (len & 0x80)
					return 0x30;
				else
					return 0x28;
			break;

			case 0x4:
				if (len & 0x80)
					return 0x60;
				else
					return 0x50;
			break;

			case 0x5:
				if (len & 0x80)
					return 0x24;
				else
					return 0x1E;
			break;

			case 0x6:
				if (len & 0x80)
					return 0x08;
				else
					return 0x07;
			break;

			case 0x7:
				if (len & 0x80)
					return 0x10;
				else
					return 0x0D;
			break;

			default:
				printf("Square1 Get Length Counter value error!\n");
			break;
		}
	} else {
		/* check bits 4 ~ 7 */
		switch (((len & 0xF0) >> 0x4)) {
			case 0x0:
				return 0x7F;
			break;

			case 0x1:
				return 0x1;
			break;

			case 0x2:
				return 0x2;
			break;

			case 0x3:
				return 0x3;
			break;

			case 0x4:
				return 0x4;
			break;

			case 0x5:
				return 0x5;
			break;

			case 0x6:
				return 0x6;
			break;

			case 0x7:
				return 0x7;
			break;

			case 0x8:
				return 0x8;
			break;

			case 0x9:
				return 0x9;
			break;

			case 0xA:
				return 0xA;
			break;

			case 0xB:
				return 0xB;
			break;

			case 0xC:
				return 0xC;
			break;

			case 0xD:
				return 0xD;
			break;

			case 0xF:
				return 0xF;
			break;

			default:
				printf("Square1 Get Length Counter value error!\n");
			break;
		}
	}
	
	return 0x0;
}


short square_mix() {
	return (square1_sample());
}
