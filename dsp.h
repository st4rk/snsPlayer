#ifndef _DSP_H_
#define _DSP_H_

#include <SDL2/SDL.h>
#include "spc700.h"

#define MAX_SAMPLES 1024

static const unsigned int modulus[] = {
            -1, 2048, 1536,
           1280, 1024,  768,
            640,  512,  384,
            320,  256,  192,
            160,  128,   96,
             80,   64,   48,
             40,   32,   24,
             20,   16,   12,
             10,    8,    6,
              5,    4,    3,
                    2,
                    1
};

static const unsigned int offset[] = {
              0,    0, 1040,
            536,    0, 1040,
            536,    0, 1040,
            536,    0, 1040,
            536,    0, 1040,
            536,    0, 1040,
            536,    0, 1040,
            536,    0, 1040,
            536,    0, 1040,
            536,    0, 1040,
                    0,
                    0
};

typedef struct dsp_channel {
	unsigned short sampleTablePtr;
	unsigned char enable;
} dsp_channel;

typedef struct channelReg {
	unsigned char VnVOLL;
	unsigned char VnVOLR;

	unsigned char VnPitchL;
	unsigned char VnPitchH;

	unsigned char VnSRCN;

	unsigned char VnADSR1;
	unsigned char VnADSR2;

	unsigned char VnGAIN;

	unsigned char VnENVX;
	unsigned char VnOUTX;
} channelReg;

typedef struct dsp {
	channelReg channelInfo[8];
	unsigned char MVOLL;
	unsigned char MVORR;
	unsigned char EVOLL;
	unsigned char EVOLR;
	unsigned char KON;
	unsigned char KOFF;
	unsigned char FLG;
	unsigned char ENDX;
	unsigned char EFB;
	unsigned char PMON;
	unsigned char NON;
	unsigned char EON;
	unsigned char DIR;
	unsigned char ESA;
	unsigned char EDL;
	unsigned char FIR[8];

	dsp_channel channel[8]; 
} dsp;


/* samples */
extern short sample[8][MAX_SAMPLES];

/* dsp */
extern dsp s_dsp;

/** dsp register write */
extern void dsp_write(unsigned char reg, unsigned char data);

/** dsp sample process */
extern void doSamples();

/** dsp register read */
extern unsigned char dsp_read(unsigned char reg);

#endif