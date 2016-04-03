#ifndef _APU_H_
#define _APU_H_

#include <SDL/SDL_mixer.h>
#include <SDL/SDL.h>
/* Square wave 1 */
#define PULSE1_DUTY_ENV     0x4000
#define PULSE1_SWEEP        0x4001
#define PULSE1_PERIOD_LOW   0x4002
#define PULSE1_LEN_PE_HIGH  0x4003
/* Square wave 2 */
#define PULSE2_DUTY_ENV     0x4004
#define PULSE2_SWEEP        0x4005
#define PULSE2_PERIOD_LOW   0x4006
#define PULSE2_LEN_PE_HIGH  0x4007
/* CPU Frequency */
#define NTSC_CPU_CLOCK      1789773
#define PAL_CPU_CLOK        1662607

/* Status */
#define STATUS_REGISTER     0x4015
/* Frame Counter */
#define FRAME_CNT_REG       0x4017


typedef struct sweep {
	unsigned char enable;
	unsigned char period;
	unsigned char neg_flag;
	unsigned char shift_cnt;
	unsigned char divider_cnt;
} sweep;

typedef struct evenlope {
	/* envelope decay loop */
	unsigned char loop_flag;
	/* envelope decay flag (disable/enable) */
	unsigned char c_flag;
	/* Volume and Envelope decay rate */
	unsigned char volume;
	/* 4 bit internal down counter */
	unsigned char down_cnt;
} envelope;

typedef struct square_wave {
	/* 11 Bit Timer used by square wave 1 and 2 */
	/* It's the wavelenght */
	unsigned short timer;
	/* duty value used by square wave 1 and 2 */
	//float duty;
	unsigned char duty;
	/* Lenght counter */
	unsigned char len_cnt;
	/* Envelope Unit */
	envelope env;
	/* Sweep Unit */
	sweep    swp;

	unsigned int out_freq;
} square_wave;


typedef struct apu_status {
	unsigned char dmc_flag;
	unsigned char noise_flag;
	unsigned char triangle_flag;
	unsigned char pulse_channel_2;
	unsigned char pulse_channel_1;
	/* 0 = 4 step mode, 1 = 5 step mode */
	unsigned char frame_cnt_mode;
	/* sequence used by the framecounter */
	unsigned int  frame_cnt_sec;
	unsigned char irq_flag;
	unsigned char dmc_interrupt;
} apu_status;


extern square_wave squareList[2];
extern apu_status  apu;
/* Square Wave 1 */
extern void square1_envelope();
extern void square1_sweep();
extern void square1_timer();
extern void square1_len_cnt();
extern void square1_freq_output();
extern short square1_sample();

/* Square Wave 2 */
extern void square2_envelope();
extern void square2_sweep();
extern void square2_len_cnt();
extern void square2_freq_output();
extern short square2_sample();

/* SDL and APU Stuff */
/* APU Lenght Counter table */
extern unsigned char square_getLenghtCnt(unsigned char len);
/* callback used to fill the audio buffer */
extern void fill_audio(void *data, Uint8* stream, int len); 
/* call used to initialize SDL Audio */
extern void open_audio();
/* call used to close SDL Audio */
extern void close_audio();
/* Mix the Square 1 and Square 2 output samples */
extern short square_mix();
/* Apu Mix */
extern void apu_mix();
#endif