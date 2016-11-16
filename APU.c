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
/* NES APU flags */
apu_status  apu;
/* Sample count used by the square and triangle waves */
unsigned int square_sample_cnt[] = {0, 0};
unsigned int triangle_sample_cnt = 0;
unsigned int noise_sample_cnt    = 0;

/*
 * This function is the Audio Call back
 * where our samples will be play
 */
void fill_audio(void *data, Uint8 *stream, int len) {
	short *buff;

	buff = (short*)stream;

	len /= 2;

	for (int i = 0; i < len; i += 2) {
		buff[i]   = square_mix();
		buff[i+1] = buff[i];
	}

}


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
	as.callback = fill_audio;

	SDL_OpenAudio(&as, NULL);
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
 * Square Wave Samples
 */
short square_sample(unsigned char unit) {
	if (squareList[unit].out_freq > 0) {
		square_sample_cnt[unit]++;

		if (square_sample_cnt[unit] >= squareList[unit].out_freq)
			square_sample_cnt[unit] = 0;

		if (square_sample_cnt[unit] < (squareList[unit].out_freq * duty_freq[squareList[unit].duty]))
			return (INTERNAL_VOLUME * squareList[unit].env.volume);
	
		return  -(INTERNAL_VOLUME * squareList[unit].env.volume);
	}
	return 0;
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
			squareList[unit].env.volume = 0;
		}

	}
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

	triangle.out_freq = (unsigned int)(NTSC_CPU_CLOCK / (32 * (triangle.timer + 1))); 
	triangle.out_freq = (unsigned int)(44100/triangle.out_freq);

}


void triangle_len_cnt() {
	/* If enable bit is set on status, force the length counter do 0 */
	if (apu.triangle_flag == 0x0) {
		/* When it reaches to 0, the sound of channel should be silenced */
		triangle.len_cnt = 0;
	} else {
		/* Check if halt flag is not set and check if the len_cnt is not already 0 */
		if (triangle.controlFlag == 0) {
			if (triangle.len_cnt > 0) 
				triangle.len_cnt--;
		}
	}
}

void triangle_linear_cnt() {
	/* check if halt flag is enable */
	if (triangle.haltFlag) {
		if (triangle.linear_cnt <= 0) {
			/** realod the linear counter */
			triangle.linear_cnt = triangle.linear;
		} else {
			/** decrement linear counter */
			triangle.linear_cnt--;
		}
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
	if (triangle.triSequence > 32) {
		triangle.triSequence = 0;
		triangle.seqValue = triangle_sequence[0];
	} else {
		triangle.seqValue = triangle_sequence[triangle.triSequence++];
	}

	triangle_freq_output();
}


/**
 * triangle timer
 */
void triangle_timer() {
	if (triangle.tmr_cnt == 0) {
		triangle.tmr_cnt = triangle.timer;
		
		if (triangle.len_cnt  && triangle.linear_cnt) {
			triangle_sequencer();
		}
	} else {
		triangle.tmr_cnt--;
	}
}


short triangle_samples() {
	triangle_timer();
	if (triangle.timer && triangle.len_cnt && triangle.linear_cnt) {
		triangle_sample_cnt++;


		if (triangle_sample_cnt >= triangle.out_freq) {
			triangle_sample_cnt = 0;
		}

		if (triangle_sample_cnt < triangle.out_freq) {
			return (INTERNAL_VOLUME * triangle.seqValue * 0x10);
		}
		
	}

	return 0;
}


/**
 * Noise Wave 
 */


/**
 * Unit Length Counter
 */
void noise_len_cnt() {
	/* If enable bit is set on status, force the length counter do 0 */
	if (apu.noise_flag == 0x0) {
		noise.len_cnt = 0;
		/* When it reaches to 0, the sound of channel should be silenced */
		noise.env.volume = 0;
	} else {
		/* Check if halt flag is not set and check if the len_cnt is not already 0 */
		if (noise.len_cnt > 0){
			noise.len_cnt--;
		}  else  {
			noise.len_cnt = 0;
		}

	}
}


/**
 * Control the wave amplitude 
 */
void noise_envelope() {
	/**
	 * verify if decay flag is not set, if it's not, should decay the volume 
	 */
	if (!(noise.env.c_flag)) {
		/**
		 * The envelope starts with volume 15, and decrements every time the unit is clocked 
		 */
		if (noise.env.volume > 0) 
			noise.env.volume--;
		else {
			/** 
			 * Check if the envelope loop flag is enable, if it's, should to load
			 * 0xF(15) to volume and count down again 
			 */
			if (noise.env.loop_flag == 1) 
				noise.env.volume = 0xF;
		}
	}
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
	noise.out_freq = (unsigned int)(NTSC_CPU_CLOCK / (32* (noise.timer + 1))); 
	noise.out_freq = (44100/noise.out_freq);
}


/**
 * Square Wave Samples
 */
short noise_samples() {
	noise_lfsr();
	if (noise.timer > 0) {
		noise_sample_cnt++;

		if (noise_sample_cnt >= noise.out_freq) {
			noise_sample_cnt = 0;
		}

		if (noise_sample_cnt < noise.out_freq) {
			return (INTERNAL_VOLUME * noise.env.volume * ((noise.lfsr & 1) * 0x5));
		}
		
		return 0;
	}
	return 0;
}

short square_mix() {
	
	float sound_mix = (float)(square_sample(SQUARE_WAVE_UNIT_1) + square_sample(SQUARE_WAVE_UNIT_2));
	sound_mix = 95.88f / (8128.0f / (sound_mix + 100.0f));

	float tnd_out = 159.79f / (1.0f / ((float)noise_samples()/12241.0f + (float)triangle_samples()/8700.0f) + 15.0f);
	sound_mix += tnd_out;

	return (short)sound_mix * 100;
}

