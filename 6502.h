#ifndef _6502_H_
#define _6502_H_

#include <stdio.h>
#include <stdlib.h>
#include "APU.h"

/* system memory */
extern unsigned char memory[65536];
/* 6502 cpu registers  */
extern unsigned char A;
extern unsigned char P;
extern unsigned char X;
extern unsigned char Y;
extern unsigned char S;
extern int PC;
extern int EA;
/* system tick count */
extern int tick_count;
/* verify if a rts was trigger */
extern unsigned char macgyver_var;
/* 6502 opcode jump table */
extern void (*op[0x100])();
/* 6502 main functions */
extern void IRQ();
extern void NMI();
extern void CPU_reset();
extern void CPU_execute(int cycles);
/* Write/Read memory */
extern unsigned char memoryRead(unsigned short addr);
extern void writeMemory(unsigned short addr, unsigned char data);

#endif