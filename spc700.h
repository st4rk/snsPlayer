#ifndef _SPC_700_H_
#define _SPC_700_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dsp.h"

#ifndef DEBUG
	#define DEBUG
#endif

/** SPC700 BootROM image */
static const unsigned char IPL_BOOTROM[] = {
	0xCD,0xEF,0xBD,0xE8,0x00,0xC6,0x1D,0xD0,0xFC,0x8F,0xAA,0xF4,0x8F,0xBB,0xF5,0x78,
	0xCC,0xF4,0xD0,0xFB,0x2F,0x19,0xEB,0xF4,0xD0,0xFC,0x7E,0xF4,0xD0,0x0B,0xE4,0xF5,
	0xCB,0xF4,0xD7,0x00,0xFC,0xD0,0xF3,0xAB,0x01,0x10,0xEF,0x7E,0xF4,0x10,0xEB,0xBA,
	0xF6,0xDA,0x00,0xBA,0xF4,0xC4,0xF4,0xDD,0x5D,0xD0,0xDB,0x1F,0x00,0x00,0xC0,0xFF
};

#define PSW_FLAG_NEGATIVE    0b10000000
#define PSW_FLAG_OVERFLOW    0b01000000
#define PSW_FLAG_DIRECTPAGE  0b00100000
#define PSW_FLAG_BREAK       0b00010000
#define PSW_FLAG_HALFCARRY   0b00001000
#define PSW_FLAG_INTENABLE   0b00000100
#define PSW_FLAG_ZERO        0b00000010
#define PSW_FLAG_CARRY       0b00000001

#define SET_FLAG(f, n) (f = f | n)
#define CLEAR_FLAG(f, n) (f = f & ~n)

typedef struct timer {
	unsigned char cnt;
	unsigned char inCnt;
	unsigned char tmr;
	unsigned char enable;
	unsigned int  cycles;
} timer;

typedef struct spc700 {
	unsigned char A;
	unsigned char X;
	unsigned char Y;
	unsigned char SP;
	unsigned char PSW;

	unsigned short PC;

	unsigned short EA;
	unsigned int cycles;

	timer tmrList[3];
	unsigned char port[4];
	unsigned char control_reg;

} spc700;

typedef struct spc_info {
	spc700 *core;
	char song_title[32];
	char game_title[32];
	char dumper_name[16];
	char comment[32];
	char dump_date[11];
	char unused[7];
	char sec_before_fout[4];
	char num_fade_ms[5];
	char sound_artist[32];
	unsigned char default_channel_disable;
	unsigned char emu_used_to_dump;
	char reserved[45];
} spc_info;

extern spc_info info;
extern spc700 spc;
extern unsigned char spc_mem[0xFFFFF];
extern unsigned char dsp_addr;
extern unsigned char dsp_data;
extern unsigned char input_ports[4];
extern unsigned char isBootROM;

/* Init CPU */
extern void spc_initCPU();
/* CPU Main Loop */
extern void spc_run();
/* Write 8 bits to memory */
extern void spc_writeMemory(unsigned short addr, unsigned char data);
/* Write 16 bits to memory */
extern void spc_writeMemory16(unsigned short addr, unsigned short data);
/* Read 8 bits from memory */
extern unsigned char spc_readMemory(unsigned short addr);
/* Read 16 bits from memory */
extern unsigned short spc_readMemory16(unsigned short addr);
/* Stack Manipulation */
extern void push(unsigned char data);
extern unsigned char pop();
/* update the timers */
extern void spc_updateTimers();

#endif
