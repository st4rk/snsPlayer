#ifndef _SPC_700_H_
#define _SPC_700_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
	unsigned char count;
	unsigned char enable;
} timer;

typedef struct spc_700 {
	/* CPU Registers */

	unsigned char A;
	unsigned char X;
	unsigned char Y;
	unsigned char SP;
	unsigned char PSW;

	unsigned short PC;

	unsigned short EA;
	unsigned int cycles;

	timer timerList[3];
} spc_700;

/* Addressing mode */
extern void immediate();
extern void directPage();
extern void directPage_indexedX();
extern void directPage_indexedY();
extern void indirect();
extern void indirect_autoIncrement();
extern void directPage_directPage();
extern void indirectPage_indirectPage();
extern void immData_directPage();
extern void directPage_bit();
extern void directPage_bitRelative();
extern void absolute_booleanBit();
extern void absolute();
extern void absolute_indexedX();
extern void absolute_indexedY();
extern void indirect_indexedX();
extern void indirect_indexedY_indirect();


/* instruction set */

extern void mov_a();
extern void mov_x();
extern void mov_y();
extern void mov_ax();
extern void mov_xa();
extern void mov_ay();
extern void mov_ya();
extern void mov_xsp();
extern void mov_spx();
extern void movw_ya();
extern void mov_dp();
extern void mov_dp_reg(unsigned char);
extern void mov_dp_x(unsigned char);
extern void mov_dp_y(unsigned char);
extern void mov_abs(unsigned char);
extern void mov_abs_x(unsigned char);
extern void mov_abs_y(unsigned char);
extern void mov_dp_ya();

/* Init CPU */
extern void spc_initCPU();
/* CPU Main Loop */
extern void spc_mainLoop();
/* Write 8 bits to memory */
extern void spc_writeMemory(unsigned short addr, unsigned char data);
/* Read 8 bits from memory */
extern unsigned char spc_readMemory(unsigned short addr);
/* Stack Manipulation */
extern void push(unsigned char data);
extern unsigned char pop();

#endif