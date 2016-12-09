/*
 * NES Audio Processing Unit
 * Written by St4rk
 * Only the two square wave channels are implemented
 */
 
#include "APU.h"


/* NES APU has two square wave channel */
square_wave squareList[2];
/* NES APU has one triangle wave channel */
triangle_wave triangle;
/* NES APU has one noise wave channel */
noise_wave noise;
/* NES APU DMC */
apu_dmc dmc;
/* NES APU flags */
apu_status  apu;
/* Sample count used by the square and triangle waves */
unsigned int square_sample_cnt[] = {0, 0};
unsigned int triangle_sample_cnt = 0;
unsigned int noise_sample_cnt    = 0;
unsigned int dmc_sample_cnt      = 0;
short samples[MAX_SAMPLES];


/*
 * This function initialize the SDL_Mixer
 */
void open_audio() {
	SDL_AudioSpec as;
	SDL_Init(SDL_INIT_AUDIO);
	as.freq     = 44100;
	as.format   = AUDIO_S16SYS;
	as.channels = 2;
	as.samples  = 256;
	as.callback = NULL;

	if (SDL_OpenAudio(&as, NULL) < 0) {
		printf("Error on SDL_OpenAudio\n");
		exit(0);
	}
	SDL_PauseAudio(0);
}

/*
 * This function stop the SDL_mixer
 */
void close_audio() {
	SDL_CloseAudio();
	SDL_Quit();
}


/**
 * Control the wave amplitude 
 */
void square_envelope(unsigned char unit) {
	/**
	 * verify if decay flag is not set, if it's not, should decay the volume 
	 */
	if (!(squareList[unit].env.c_flag)) {
		/**
		 * The envelope starts with volume 15, and decrements every time the unit is clocked 
		 */
		if (squareList[unit].env.volume > 0) 
			squareList[unit].env.volume--;
		else {
			/** 
			 * Check if the envelope loop flag is enable, if it's, should to load
			 * 0xF(15) to volume and count down again 
			 */
			if (squareList[unit].env.loop_flag == 1) 
				squareList[unit].env.volume = 0xF;
		}
	}
}


/**
 * Calculate the output frequency
 */
void square_freq_output(unsigned char unit) {
	/**
	 * The output frequency of the generator can be determined by the timer(period)
	 * if the timer has a value < 8, the channel is silenced 
	 */

	if (squareList[unit].timer > 8) {
		squareList[unit].out_freq = (unsigned int)(NTSC_CPU_CLOCK / (16 * (squareList[unit].timer + 1))); 
		squareList[unit].out_freq = (44100/squareList[unit].out_freq);
	} else {
		squareList[unit].out_freq = 0;
	}

}

/**
 * Control wave period 
 */
void square_sweep(unsigned char unit) {
	unsigned short newPeriod = 0x0;
	if (squareList[unit].swp.enable) {
		/* if the divider counter is not equal 0, decrement */
		if (squareList[unit].swp.divider_cnt > 0)
			squareList[unit].swp.divider_cnt--;
		else {
			/* it's equal a 0, the counter should be set with P */
			squareList[unit].swp.divider_cnt = squareList[unit].swp.period;
			/* the pulse's period is adjusted */

			/* Verify if timer >= 8, otherwise the channel should be silenced */
			if (squareList[unit].timer >= 8) {
				/* Verify if negated flag is enable, if yes, you should decrement the 
				shift value from timer, on channel 2, you should increment + 1 to the final result*/
				
				
				/* a negative sweep on pulse channel 1 will subtract the shifted period value minus 1. */
				if (squareList[unit].swp.neg_flag) 
					newPeriod = (squareList[unit].timer - (squareList[unit].timer >> squareList[unit].swp.shift_cnt) - 1);
				else
					newPeriod = (squareList[unit].timer + (squareList[unit].timer >> squareList[unit].swp.shift_cnt));
			

				/* if the target period (current period(timer) + shift result) is greater than 0x7FF
				the channel should be silenced */
				if (newPeriod > 0x7FF) {
					squareList[unit].env.volume = 0;
				} else {
					if ((squareList[unit].swp.enable) && (squareList[unit].swp.shift_cnt != 0)) {
						/* Store new period and update the frequency */
						squareList[unit].timer = newPeriod;
						square_freq_output(unit); 
					}
				}

			} else {
				squareList[unit].env.volume = 0;
			}
		}
	}
}

/**
 * Unit Length Counter
 */
void square_len_cnt(unsigned char unit) {
	/* If enable bit is set on status, force the length counter do 0 */
	if (apu.pulse_channel_1 == 0x0) {
		squareList[unit].len_cnt = 0;
		/* When it reaches to 0, the sound of channel should be silenced */
		squareList[unit].env.volume = 0;
	} else {
		/* if the len_cnt reachs to 0, silence the channel */
		if (squareList[unit].len_cnt > 0) {
			squareList[unit].len_cnt--;
		} else {
			squareList[unit].len_cnt = 0;
		}

	}
}

/**
 * Square Wave Samples
 */
short square_sample(unsigned char unit) {
	square_sweep(unit);

	if (squareList[unit].out_freq && squareList[unit].timer) {
		square_sample_cnt[unit]++;

		if (square_sample_cnt[unit] >= squareList[unit].out_freq)
			square_sample_cnt[unit] = square_sample_cnt[unit] - squareList[unit].out_freq;

		if (square_sample_cnt[unit] < (squareList[unit].out_freq * duty_freq[squareList[unit].duty]))
			return (squareList[unit].env.volume);
	
		return  -(squareList[unit].env.volume);
	}
	return 0;
}


/* APU Lenght Count Table */
unsigned char getLengthCnt(unsigned char len) {
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


/*
 *  Triangle Wave Channel
 */
void triangle_freq_output() {
	float out_freq = (NTSC_CPU_CLOCK / (32 * (triangle.timer + 1)));
	out_freq = 44100.0f / out_freq;

	triangle.out_freq = (unsigned int)(out_freq);
}


void triangle_len_cnt() {

	/* Check if halt flag is not set and check if the len_cnt is not already 0 */
	if (!triangle.controlFlag) {
		if (triangle.len_cnt > 0) {
			triangle.len_cnt--;
		} else {
			triangle.len_cnt = 0;	
		}
	}

}

void triangle_linear_cnt() {

	/* check if halt flag is enable */
	if (triangle.haltFlag) {
		triangle.linear_cnt = triangle.linear;
	} else {
		if (triangle.linear_cnt > 0)
			--triangle.linear_cnt;
	}

	/** if control flag is clear, clear halt flag */
	if (!triangle.controlFlag) {
		triangle.haltFlag = 0;
	}


}

/**
 * triangle sequencer
 */
void triangle_sequencer() {
	triangle.seqValue = triangle_sequence[triangle.triSequence++ & 31];
}


/**
 * triangle timer
 */
void triangle_timer() {
	if ((triangle.linear_cnt > 0) && (triangle.len_cnt > 0)) {
		if (triangle.tmr_cnt) {
			triangle.tmr_cnt--;
		} else {
			triangle.tmr_cnt = triangle.timer;
			triangle_sequencer();
		}
	}
}


short triangle_samples() {
	if ((triangle.len_cnt) && (triangle.linear_cnt)) {
		triangle_sample_cnt++;

		if (triangle_sample_cnt > triangle.out_freq) {
			triangle_sample_cnt = triangle_sample_cnt - triangle.out_freq;
		}

		triangle.oldOutput = triangle.seqValue;

		if (triangle.timer < 2) {
			triangle.oldOutput = triangle_sequence[7] * 2;
		}

		return triangle.oldOutput * 2;
	
	}

	return triangle.oldOutput;
}


/**
 * Noise Wave 
 */


/**
 * Unit Length Counter
 */
void noise_len_cnt() {

	if (noise.env.loop_flag) {
		/* Check if halt flag is not set and check if the len_cnt is not already 0 */
		if (noise.len_cnt > 0){
			noise.len_cnt--;
		}  else  {
			noise.len_cnt = 0;
			noise.env.volume = 0;
		}
	} else {
		noise.env.volume = 0;
	}
}


/**
 * Control the wave amplitude 
 */
void noise_envelope() {
	if (noise.env.c_flag) 
		noise.env.volume > 0 ? noise.env.volume-- : noise.env.loop_flag == 0 ? 15 : 0;
}


/**
 * Noise Linear feedback shift register
 */
void noise_lfsr() {
	unsigned char mode = noise.mode == 0 ? 13 : 8;


	unsigned short bit = (noise.lfsr >> 14) ^ (noise.lfsr >> mode);
	bit = (bit | 0x4000);
	noise.lfsr = (noise.lfsr << 1) | (bit & 0x1);
	noise_out_freq();
}

/**
 * Noise timer
 */
void noise_timer() {
	if (noise.tmr_cnt == 0) {
		noise.tmr_cnt = noise.timer;
		noise_lfsr();
	} else {
		noise.tmr_cnt--;
	}
}

/**
 * Calculate the output frequency
 */
void noise_out_freq() {
	noise.out_freq = (unsigned int)(NTSC_CPU_CLOCK /  (16 * noise.timer + 1)); 
	noise.out_freq = (44100/noise.out_freq);
}


/**
 * Square Wave Samples
 */
short noise_samples() {
	if (noise.len_cnt) {
		noise_sample_cnt++;

		if (noise_sample_cnt > noise.out_freq) {
			noise_sample_cnt -= noise.out_freq;
		}

		return (noise.env.volume * ((noise.lfsr & 1) * 4));
	
	}
	return 0;
}


/**
 * DMC Channel 
 */

void dmc_update_freq() {
	dmc.out_freq = (unsigned int)(NTSC_CPU_CLOCK /  (16 * dmc.freq + 1)); 
	dmc.out_freq = (44100/dmc.out_freq);
}


short dmc_samples() {
	dmc_sample_cnt++;

	if (dmc_sample_cnt > dmc.out_freq) {
		dmc_sample_cnt -= dmc.out_freq;

		if (dmc.sizeCnt && dmc.shift) {
			unsigned short oldAddr = dmc.addrCnt;

			if (dmc.addrCnt == 0xFFFF) {
				dmc.addrCnt = 0x8000;
			} else {
				dmc.addrCnt++;
			}

			dmc.directLoad = memoryRead(oldAddr);
			dmc.sizeCnt--;
			dmc.shift = 8;

			if (dmc.loop && (dmc.sizeCnt <= 0)) {
				dmc.sizeCnt = dmc.size;
				dmc.addrCnt = dmc.addr;
			}
		}


		if (dmc.sizeCnt > 0) {
			if (dmc.directLoad) {
				if (!(dmc.directLoad & 0x1) && (dmc.dacCnt > 1)) {
					dmc.dacCnt -= 2;
				} else if ((dmc.directLoad & 0x1) && (dmc.dacCnt < 0x7E)) {
					dmc.dacCnt += 2;
				}
			}

			dmc.dacCnt--;

			if (dmc.dacCnt == 0) dmc.dacCnt = 8;

			dmc.directLoad >>= 1;
			dmc.shift--;
		}
	}

	return dmc.dacCnt;
}

short mix_channel() {
	
	float sound_mix = 0.00752f * (float)(square_sample(SQUARE_WAVE_UNIT_1) + square_sample(SQUARE_WAVE_UNIT_2));

//	float tnd_out = 159.79f / (1.0f / ((float)(triangle_samples()/8227.0f) + (float)noise_samples()/12241.0f) + 400.0f);
// 0.00851f * (float)triangle_samples(); //
	float tnd_out =  0.00851f * (float)triangle_samples() + 0.00494f * (float)noise_samples();
	sound_mix += tnd_out;
	//float sound_mix = (float)dmc_samples();

	return (short)(sound_mix * 50000);
}

