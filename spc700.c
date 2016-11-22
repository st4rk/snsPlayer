#include "spc700.h"

/* SPU */

spc700 spc;
spc_info info;

/* Internal Memory 64 KB */
unsigned char spc_mem[0xFFFFF];
/* DSP Address */
unsigned char dsp_addr;
/* DSP Data */
unsigned char dsp_data;

/* Write 8 bits to memory */
void spc_writeMemory(unsigned short addr, unsigned char data) {
#ifdef DEBUG
//	printf("Debug - Write Addr: 0x%X   - Data: 0x%X\n", addr, data);
#endif

	/* Registers */
	if ((addr >= 0x00F0) && (addr <= 0x00FF)) {	
		switch (addr & 0xFF) {
			/** TEST testing functions */
			case 0xF0:
#ifdef DEBUG
			printf("Trying to write test register\n");
#endif
			break;
			/** control register */
			case 0xF1:
				spc.control_reg = data;

				/* clear input ports 0xF4 and 0xF5 */
				if (data & 0x10) {
					spc.port[0] = 0x0;
					spc.port[1] = 0x0;
				}

				/* clear input ports 0xF6 and 0xF7 */
				if (data & 0x20) {
					spc.port[2] = 0x0;
					spc.port[3] = 0x0;
				}

				/* verify if need to enable timer register */
				spc.tmrList[0].enable = (data & 0x1);
				spc.tmrList[1].enable = (data & 0x2);
				spc.tmrList[2].enable = (data & 0x4);
			break;
			/* DSP register address */
			case 0xF2:
#ifdef DEBUG
				printf("DSP Register Addr: 0x%X\n", data);
#endif
				dsp_addr = data;
			break;
			/* DSP register data */
			case 0xF3:
#ifdef DEBUG
				printf("DSP Register Data: 0x%X\n", data);
#endif
				dsp_write(dsp_addr, data);
			break;
			/* port 0 */
			case 0xF4:
				spc.port[0] = data;
			break;
			/* port 1 */
			case 0xF5:
				spc.port[1] = data;
			break;
			/* port 2 */
			case 0xF6:
				spc.port[2] = data;
			break;
			/* port 3 */
			case 0xF7:
				spc.port[3] = data;
			break;
			/* regular memory */
			case 0xF8:
				spc_mem[addr] = data;
			break;
			/* regular memory */
			case 0xF9:
				spc_mem[addr] = data;
			break;
			/* timer 0 */
			case 0xFA:
				spc.tmrList[0].tmr = data;
			break;
			/* timer 1 */
			case 0xFB:
				spc.tmrList[1].tmr = data;
			break;
			/* timer 2 */
			case 0xFC:
				spc.tmrList[2].tmr = data;
			break;
			/* counter 0 */
			case 0xFD:

			break;
			/* counter 1 */
			case 0xFE:

			break;
			/* counter 2 */
			case 0xFF:

			break;
		}
	} 
	
	spc_mem[addr] = data;
}

/* Read 8 bits from memory */
unsigned char spc_readMemory(unsigned short addr) {
	unsigned char data = 0;
#ifdef DEBUG
	//printf("Debug - Read Addr: 0x%X\n", addr);
#endif

	/* Registers */
	if ((addr >= 0x00F0) && (addr <= 0x00FF)) {
		switch (addr) {
			/** TEST testing functions */
			case 0xF0:
#ifdef DEBUG
			printf("Trying to read test register\n");
#endif
			break;
			/** control register */
			case 0xF1:
				return spc.control_reg;
			break;
			/* DSP register address */
			case 0xF2:
#ifdef DEBUG
				printf("DSP Register Addr: 0x%X\n", dsp_addr);
#endif
				return dsp_read(dsp_addr);
			break;
			/* DSP register data */
			case 0xF3:
#ifdef DEBUG
				printf("DSP Register Data: 0x%X\n", dsp_data);
#endif
				return dsp_read(dsp_addr);
			break;
			/* port 0 */
			case 0xF4:
				return spc.port[0];
			break;
			/* port 1 */
			case 0xF5:
				return spc.port[1];
			break;

			/* port 2 */
			case 0xF6:
				return spc.port[2];
			break;

			/* port 3 */
			case 0xF7:
				return spc.port[3];
			break;

			/* regular memory */
			case 0xF8:
				return spc_mem[addr];
			break;
			/* regular memory */
			case 0xF9:
				return spc_mem[addr];
			break;
			/* timer 0 */
			case 0xFA:

			break;
			/* timer 1 */
			case 0xFB:

			break;
			/* timer 2 */
			case 0xFC:

			break;
			/* counter 0 */
			case 0xFD:
				data = spc.tmrList[0].cnt;
				spc.tmrList[0].cnt = 0;
				return data;
			break;
			/* counter 1 */
			case 0xFE:
				data = spc.tmrList[1].cnt;
				spc.tmrList[1].cnt = 0;
				return data;
			break;
			/* counter 2 */
			case 0xFF:
				data = spc.tmrList[2].cnt;
				spc.tmrList[2].cnt = 0;
				return data;
			break;
		}
	}

	if ((addr >= 0xFFC0) && (addr <= 0xFFFF)) {
		if (spc.control_reg & 0x80) {
			return IPL_BOOTROM[addr & 0x3F];
		} else {
			return spc_mem[addr];
		}
	}

	return spc_mem[addr];

}

void spc_writeMemory16(unsigned short addr, unsigned short data) {

}

/* Stack Manipulation */
void push(unsigned char data) {
	spc_writeMemory((spc.SP | 0x0100), data);

	spc.SP--;
}

unsigned char pop() {
	spc.SP++;

	return (spc_readMemory((spc.SP | 0x0100)));
}

/* Addressing mode */

void immediate() {
	spc.EA = spc.PC++;
}

void directPage() {
	spc.EA = spc_readMemory(spc.PC++) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3);
}

void directPage_indexedX() {
	directPage();
	unsigned short highByte = (spc.EA & 0xFF00);
	unsigned short addr = spc.EA + spc.X;
	spc.EA = (highByte) | (addr & 0xFF);
}

void directPage_indexedY()  {
	directPage();
	unsigned short highByte = (spc.EA & 0xFF00);
	unsigned short addr = spc.EA + spc.Y;
	spc.EA = (highByte) | (addr & 0xFF);
}

void indirect() {
	spc.EA = (spc_readMemory(spc.X | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3)));
}

void indirect_autoIncrement() {
	spc.EA = (spc_readMemory(spc.X++ | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3)));
}

void absolute() {
	spc.EA = (spc_readMemory(spc.PC) | (spc_readMemory(spc.PC + 1) << 8));
	spc.PC += 2;
}

void absolute_indexedX() {
	spc.EA = (spc_readMemory(spc.PC) | (spc_readMemory(spc.PC + 1) << 8)) + spc.X;
	spc.PC += 2;
}

void absolute_indexedY() {
	spc.EA = (spc_readMemory(spc.PC) | (spc_readMemory(spc.PC + 1) << 8)) + spc.Y;
	spc.PC += 2;
}

void indirect_indexedX() {
	directPage_indexedX();
	spc.EA = (spc_readMemory(spc.EA) | (spc_readMemory(spc.EA + 1) << 8));
}

void indirect_indexedY_indirect() {
	directPage();
	spc.EA = (spc_readMemory(spc.EA) | (spc_readMemory(spc.EA + 1) << 8)) + spc.Y;
}

/* Instruction set */

void mov_a() {
	spc.A = spc_readMemory(spc.EA);

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void mov_x() {
	spc.X = spc_readMemory(spc.EA);

	if (spc.X) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.X & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void mov_y() {
	spc.Y = spc_readMemory(spc.EA);

	if (spc.Y) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.Y & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void mov_ax() {
	spc.A = spc.X;

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void mov_xa() {
	spc.X = spc.A;

	if (spc.X) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.X & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void mov_ay() {
	spc.A = spc.Y;
	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void mov_ya() {
	spc.Y = spc.A;
	if (spc.Y) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.Y & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void mov_xsp() {
	spc.X = spc.SP;
	if (spc.X) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.X & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void mov_spx() {
	spc.SP = spc.X;
}

void movw_ya() {
	unsigned short YA = (spc_readMemory(spc.EA) | (spc_readMemory(spc.EA + 1) << 8));

	spc.A = spc_readMemory(spc.EA);
	spc.Y = spc_readMemory(spc.EA + 1);

	if (YA) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (YA & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void mov_dp() {
	unsigned char data =  spc_readMemory(spc.EA);
	directPage();
	spc_writeMemory(spc.EA, data);
}

void mov_dp_reg(unsigned char reg) {
	directPage();
	spc_writeMemory(spc.EA, reg);
}

void mov_dp_x(unsigned char reg) {
	directPage_indexedX();
	spc_writeMemory(spc.EA, reg);
}

void mov_dp_y(unsigned char reg) {
	directPage_indexedY();
	spc_writeMemory(spc.EA, reg);
}

void mov_abs(unsigned char reg) {
	absolute();
	spc_writeMemory(spc.EA, reg);
}

void mov_abs_x(unsigned char reg) {
	absolute_indexedX();
	spc_writeMemory(spc.EA, reg);
}

void mov_abs_y(unsigned char reg) {
	absolute_indexedY();
	spc_writeMemory(spc.EA, reg);
}

void mov_dp_ya() {
	spc_writeMemory(spc.EA, spc.A);
	spc_writeMemory(spc.EA+1, spc.Y);
}

void or() {
	spc.EA = spc_readMemory(spc.EA);

	spc.A = (spc.A | (spc.EA & 0xFF));

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void or_xy() {
	unsigned short X = (spc.X | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
	unsigned short Y = (spc.Y | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));

	unsigned char X_2 = spc_readMemory(X);
	Y = spc_readMemory(Y);

	X_2 = (X_2 | Y);

	spc_writeMemory(X, X_2);

	if (X_2) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (X_2 & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void or_dp() {
	unsigned char data_1 = spc_readMemory(spc.EA);
	directPage();
	unsigned char data_2 = spc_readMemory(spc.EA);

	data_2 = (data_2 | data_1);

	spc_writeMemory(spc.EA, data_2);

	if (data_2) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data_2 & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void and() {
	spc.EA = spc_readMemory(spc.EA);

	spc.A = (spc.A & (spc.EA & 0xFF));

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void and_xy() {
	unsigned short X = (spc.X | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
	unsigned short Y = (spc.Y | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));

	unsigned char X_2 = spc_readMemory(X);
	Y = spc_readMemory(Y);

	X_2 = (X_2 & Y);

	spc_writeMemory(X, X_2);


	if (X_2) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (X_2 & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void and_dp() {
	unsigned char data_1 = spc_readMemory(spc.EA);
	directPage();
	unsigned char data_2 = spc_readMemory(spc.EA);

	data_2 = (data_2 & data_1);

	spc_writeMemory(spc.EA, data_2);

	if (data_2) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data_2 & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void eor() {
	spc.EA = spc_readMemory(spc.EA);

	spc.A = (spc.A ^ (spc.EA & 0xFF));

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void eor_xy() {
	unsigned short X = (spc.X | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
	unsigned short Y = (spc.Y | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));

	unsigned char X_2 = spc_readMemory(X);
	Y = spc_readMemory(Y);

	X_2 = (X_2 ^ Y);

	spc_writeMemory(X, X_2);

	if (X_2) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (X_2 & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void eor_dp() {
	unsigned char data_1 = spc_readMemory(spc.EA);
	directPage();
	unsigned char data_2 = spc_readMemory(spc.EA);

	data_2 = (data_2 ^ data_1);

	spc_writeMemory(spc.EA, data_2);

	if (data_2) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data_2 & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void adc() {
	unsigned char data = spc_readMemory(spc.EA);
	unsigned short temp = (data + spc.A + (spc.PSW & PSW_FLAG_CARRY));

	/* Verify if a carry happened */
	if (temp > 0xFF) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* verify if a half flag happened */
	if ((temp ^ data ^ spc.A) & 0x10) SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);

	/* Check a overflow */
	if ((~(spc.A ^ data)) & (spc.A ^ temp) & 0x80) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);

	spc.A = (temp & 0xFF);

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void adc_imm() {
	unsigned char data_1 = spc_readMemory(spc.EA);
	directPage();
	unsigned char data_2 = spc_readMemory(spc.EA);

	unsigned short temp = (data_1 + data_2 + (spc.PSW & PSW_FLAG_CARRY));

	/* verify if a half flag happened */
	if ((temp ^ data_1 ^ data_2) & 0x10) SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);

	/* Verify if a carry happened */
	if (temp > 0xFF) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* Check a overflow */
	if ((~(data_1 ^ data_2)) & (data_2 ^ temp) & 0x80) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);

	temp = temp & 0xFF;
	spc_writeMemory(spc.EA, temp);

	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void adc_xy() {
	unsigned short X = (spc.X | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
	unsigned short Y = (spc.Y | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));

	unsigned char X_2 = spc_readMemory(X);
	Y = spc_readMemory(Y);

	unsigned short temp = (X_2 + Y + (spc.PSW & PSW_FLAG_CARRY));

	/* Verify if a carry happened */
	if (temp > 0xFF) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* verify if a half flag happened */
	if ((temp ^ X_2 ^ Y) & 0x10) SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);

	/* Check a overflow */
	if ((~(X_2 ^ Y)) & (X_2 ^ temp) & 0x80) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);

	temp = temp & 0xFF;
	spc_writeMemory(X, temp);

	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void adc_dp() {
	unsigned char data_1 = spc_readMemory(spc.EA);
	directPage();
	unsigned char data_2 = spc_readMemory(spc.EA);

	unsigned short temp = ((data_1 + data_2 + (spc.PSW & PSW_FLAG_CARRY)));

	/* verify if a half flag happened */
	if ((temp ^ data_1 ^ data_2) & 0x10) SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);

	/* Verify if a carry happened */
	if (temp > 0xFF) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* Check a overflow */
	if ((~(data_2 ^ data_1)) & (data_2 ^ temp) & 0x80) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);

	temp = temp & 0xFF;
	spc_writeMemory(spc.EA, temp);

	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void sbc() {
	unsigned char data = spc_readMemory(spc.EA);
	int temp = (data - spc.A - (1 - (spc.PSW & PSW_FLAG_CARRY)));

	/* Verify if a carry happened */
	if (temp >= 0x0) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* verify if a half flag happened */
	if (((temp ^ data ^ spc.A) & 0x10) == 0) SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);

	/* Check a overflow */
	if (((spc.A ^ data)) & (spc.A ^ temp) & 0x80) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);

	spc.A = (temp & 0xFF);

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void sbc_imm() {
	unsigned char data_1 = spc_readMemory(spc.EA);
	directPage();
	unsigned char data_2 = spc_readMemory(spc.EA);

	int temp = (data_1 - data_2 - (1 - (spc.PSW & PSW_FLAG_CARRY)));

	/* Verify if a carry happened */
	if (temp >= 0x0) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* verify if a half flag happened */
	if (((temp ^ data_1 ^ data_2) & 0x10) == 0) SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);

	/* Check a overflow */
	if (((data_2 ^ data_1)) & (data_2 ^ temp) & 0x80) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);

	temp = temp & 0xFF;
	spc_writeMemory(spc.EA, temp);

	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void sbc_xy() {
	unsigned short X = (spc.X | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
	unsigned short Y = (spc.Y | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));

	unsigned char X_2 = spc_readMemory(X);
	Y = spc_readMemory(Y);

	int temp = (X_2 - Y - (1 - (spc.PSW & PSW_FLAG_CARRY)));

	/* verify if a half flag happened */
	if (((temp ^ X_2 ^ Y) & 0x10) == 0) SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);

	/* Verify if a carry happened */
	if (temp >= 0x0) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* Check a overflow */
	if (((X_2 ^ Y)) & (X_2 ^ temp) & 0x80) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);

	temp = temp & 0xFF;
	spc_writeMemory(X, temp);

	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void sbc_dp() {
	unsigned char data_1 = spc_readMemory(spc.EA);
	directPage();
	unsigned char data_2 = spc_readMemory(spc.EA);

	int temp = (data_1 - data_2 - (1 - (spc.PSW & PSW_FLAG_CARRY)));

	/* Verify if a carry happened */
	if (temp >= 0x0) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* verify if a half flag happened */
	if (((temp ^ data_1 ^ data_2) & 0x10) == 0) SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);

	/* Check a overflow */
	if (((data_2 ^ data_1)) & (data_2 ^ temp) & 0x80) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);

	temp = temp & 0xFF;
	spc_writeMemory(spc.EA, temp);

	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void cmp() {
	spc.EA = spc_readMemory(spc.EA);

	int temp = spc.A - spc.EA;

	if (temp >= 0) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	temp = temp & 0xFF;
	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void cmp_xy() {
	unsigned short X = (spc.X | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
	unsigned short Y = (spc.Y | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));

	X = spc_readMemory(X);
	Y = spc_readMemory(Y);

	int temp = X - Y;

	if (temp >= 0) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	temp = temp & 0xFF;
	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void cmp_dp() {
	unsigned char data = spc_readMemory(spc.EA);
	directPage();
	spc.EA = spc_readMemory(spc.EA);

	int temp = spc.EA - data;

	if (temp >= 0) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	temp = temp & 0xFF;
	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void cpx() {
	spc.EA = spc_readMemory(spc.EA);

	int temp = spc.X - spc.EA;

	if (temp >= 0) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	temp = temp & 0xFF;
	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void cpy() {
	spc.EA = spc_readMemory(spc.EA);

	int temp = spc.Y - spc.EA;

	if (temp >= 0) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	temp = temp & 0xFF;
	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void inc_mem() {
	unsigned char data = spc_readMemory(spc.EA);

	data = data + 1;

	spc_writeMemory(spc.EA, data);

	if (data) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void inc_reg(unsigned char *reg) {

	*reg = *reg + 1;

	if (*reg) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (*reg & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void dec_mem() {
	unsigned char data = spc_readMemory(spc.EA);

	data = data - 1;

	spc_writeMemory(spc.EA, data);
	if (data) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void dec_reg(unsigned char *reg) {
	*reg = *reg - 1;

	if (*reg) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (*reg & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void asl_a() {
	if (spc.A & 0x80)  SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	spc.A <<= 1;

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void asl_mem() {
	unsigned char data = spc_readMemory(spc.EA);

	if (data & 0x80) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	data <<= 1;

	spc_writeMemory(spc.EA, data);

	if (data) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void lsr_a() {
	if (spc.A & 0x1) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	spc.A >>= 1;

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void lsr_mem() {
	unsigned char data = spc_readMemory(spc.EA);

	if (data & 0x1) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	data >>= 1;

	spc_writeMemory(spc.EA, data);

	if (data) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void rol_a() {
	unsigned char c = spc.PSW & PSW_FLAG_CARRY;

	if (spc.A & 0x80)  SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	spc.A = (spc.A << 1) | c;

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}

void rol_mem() {
	unsigned char data = spc_readMemory(spc.EA);
	unsigned char c = spc.PSW & PSW_FLAG_CARRY;

	if (data & 0x80) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	data = (data << 1) | c;

	spc_writeMemory(spc.EA, data);

	if (data) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void ror_a() {
	unsigned char c = spc.PSW & PSW_FLAG_CARRY;

	if (spc.A & 0x1)  SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	spc.A = (spc.A >> 1) | c;

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}

void ror_mem() {
	unsigned char data = spc_readMemory(spc.EA);
	unsigned char c = spc.PSW & PSW_FLAG_CARRY;

	if (data & PSW_FLAG_CARRY) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	data = (data >> 1) | c;

	spc_writeMemory(spc.EA, data);

	if (data) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void incw() {
	unsigned short data = (spc_readMemory(spc.EA) | (spc_readMemory(spc.EA + 1) << 8));

	data++;

	spc_writeMemory(spc.EA, data & 0xFF);
	spc_writeMemory(spc.EA+1, (data >> 8) & 0xFF);

	if (data) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void decw() {
	unsigned short data = (spc_readMemory(spc.EA) | (spc_readMemory(spc.EA + 1) << 8));

	data--;

	spc_writeMemory(spc.EA, data & 0xFF);
	spc_writeMemory(spc.EA+1, (data >> 8) & 0xFF);

	if (data) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void addw() {
	unsigned short data = (spc_readMemory(spc.EA) | (spc_readMemory(spc.EA + 1) << 8));
	unsigned short YA   = spc.A | (spc.Y << 8);
	unsigned int result = YA + data;

	if ((~(YA ^ data)) & (YA ^ result) & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);
	if ((YA ^ data ^ result) & 0x1000) SET_FLAG (spc.PSW, PSW_FLAG_HALFCARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
	if (result > 0xFFFF) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	result = result & 0xFFFF;
	spc.Y = (result >> 0x8) & 0xFF;
	spc.A = (result & 0xFF);

	if (result) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (result & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void subw() {
	unsigned short data = (spc_readMemory(spc.EA) | (spc_readMemory(spc.EA + 1) << 8));
	unsigned short YA   = spc.A | (spc.Y << 8);
	int result = YA - data;

	if ((YA ^ data) & (YA ^ result) & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);
	if (((YA ^ data ^ result) & 0x1000) == 0) SET_FLAG (spc.PSW, PSW_FLAG_HALFCARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
	if (result >= 0x0) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	result = result & 0xFFFF;
	spc.Y = (result >> 0x8) & 0xFF;
	spc.A = (result & 0xFF);

	if (result) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (result & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void cmpw() {
	unsigned short data = (spc_readMemory(spc.EA) | (spc_readMemory(spc.EA + 1) << 8));
	unsigned short YA   = spc.A | (spc.Y << 8);

	YA = YA - data;

	if (YA >= 0) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	if (YA) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (YA & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void bbc(unsigned char bit_mask) {
	unsigned char dp = spc_readMemory(spc.EA);
    char rel = spc_readMemory(spc.PC++);

	if (!(dp & bit_mask)) {
		 spc.PC += rel;
		 spc.cycles += 2;
	}
}

void bbs(unsigned char bit_mask) {
	unsigned char dp = spc_readMemory(spc.EA);
	char rel = spc_readMemory(spc.PC++);

	if (dp & bit_mask) {
		spc.PC += rel;
		spc.cycles += 2;
	}
}

void bcc() {
	char rel = spc_readMemory(spc.PC++);

	if (!(spc.PSW & PSW_FLAG_CARRY)) {
		spc.PC += rel;
		spc.cycles += 2;
	}
}

void bcs() {
	char rel = spc_readMemory(spc.PC++);

	if (spc.PSW & PSW_FLAG_CARRY) {
		spc.PC += rel;
		spc.cycles += 2;
	}
}

void beq() {
	char rel = spc_readMemory(spc.PC++);

	if (spc.PSW & PSW_FLAG_ZERO) {
		spc.PC += rel;
		spc.cycles += 2;
	}
}

void bmi() {
	char rel = spc_readMemory(spc.PC++);

	if (spc.PSW & PSW_FLAG_NEGATIVE) {
		spc.PC += rel;
		spc.cycles += 2;
	}
}

void bne() {
	char rel = spc_readMemory(spc.PC++);

	if (!(spc.PSW & PSW_FLAG_ZERO)) {
		spc.PC += rel;
		spc.cycles += 2;
	}
}

void bpl() {
	char rel = spc_readMemory(spc.PC++);

	if (!(spc.PSW & PSW_FLAG_NEGATIVE)) {
		spc.PC += rel;
		spc.cycles += 2;
	}
}

void bvc() {
	char rel = spc_readMemory(spc.PC++);

	if (!(spc.PSW & PSW_FLAG_OVERFLOW)) {
		spc.PC += rel;
		spc.cycles += 2;
	}
}

void bvs() {
	char rel = spc_readMemory(spc.PC++);

	if (spc.PSW & PSW_FLAG_OVERFLOW) {
		spc.PC += rel;
		spc.cycles += 2;
	}
}

void bra() {
	char rel = spc_readMemory(spc.PC++);
	spc.PC += rel;
}

void brk() {
	 spc.PC++;
	 push((spc.PC & 0xFF00) >> 8);
 	 push(spc.PC & 0xFF);
	 push(spc.PSW);
	 spc.PC = (spc_readMemory(0xFFDE) | (spc_readMemory(0xFFDF) << 8));
	 SET_FLAG(spc.PSW, PSW_FLAG_BREAK);
	 CLEAR_FLAG(spc.PSW, PSW_FLAG_INTENABLE);
}

void call() {
	push(spc.PC  >> 8);
	push(spc.PC & 0xFF);
	spc.PC = spc.EA;
}

void cbne() {
	unsigned char data = spc_readMemory(spc.EA);
	unsigned char rel = spc_readMemory(spc.PC++);

	if (data != spc.A) {
		spc.PC += rel;
		spc.cycles += 2;
	}
}

void clr1(unsigned char bit_mask) {
	spc_writeMemory(spc.EA, (spc_readMemory(spc.EA) & ~bit_mask));
}

void clrc() {
	CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
}

void clrp() {
	CLEAR_FLAG(spc.PSW, PSW_FLAG_DIRECTPAGE);
}

void clrv() {
	CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);
	CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
}

void di() {
	CLEAR_FLAG(spc.PSW, PSW_FLAG_INTENABLE);
}

void ei() {
	SET_FLAG(spc.PSW, PSW_FLAG_INTENABLE);
}

void dbnz_y() {
	char rel = spc_readMemory(spc.PC++);
	spc.Y--;

	if (spc.Y != 0) {
		spc.PC += rel;
		spc.cycles += 2;
	}
}

void dbnz_d() {
	directPage();
	unsigned char data = spc_readMemory(spc.EA);
	char rel = spc_readMemory(spc.PC++);

	data = (data - 1) & 0xFF;

	spc_writeMemory(spc.EA, data);

	if (data != 0) {
		spc.PC += rel;
		spc.cycles += 2;
	}
}

void DIV() {
	unsigned short YA = spc.A | (spc.Y << 8);

	unsigned int yva = YA;
	unsigned int xva = (spc.X << 9);

	for (int i = 0; i < 9; i++) {
		yva = yva << 1;
		if (yva & 0x20000) yva = (yva & 0x1FFFF) | 0x1;
		if (yva >= xva) yva = yva ^ 0x1;
		if (yva & 0x1) yva = (yva - xva) & 0x1FFFF;
	}
	/**  yva => Y, A, and V flag as YYYYYYYY V AAAAAAAA src: Anomie's SPC700 Doc */
	spc.Y = (yva >> 0x9) & 0xFF;
	spc.A = yva & 0xFF;

	if (yva & 0x100)  SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);
	if ((spc.X & 0xF) <= (spc.Y & 0xF)) SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void jmp_1f() {
	unsigned short data = spc_readMemory(spc.EA) | (spc_readMemory(spc.EA + 1) << 8);
	spc.PC = data;
}

void jmp_5f() {
	spc.PC = spc.EA;
}

void notc() {
	spc.PSW = spc.PSW ^ PSW_FLAG_CARRY;
}

void ret() {
	spc.PC = pop();
	spc.PC = spc.PC | (pop() << 8);
}

void ret1() {
	spc.PSW = pop();
	spc.PC = pop();
	spc.PC = spc.PC | (pop() << 8);
}

void set1(unsigned char bit_mask) {
	spc_writeMemory(spc.EA, (spc_readMemory(spc.EA) | bit_mask));
}

void setPSW(unsigned char flag) {
	SET_FLAG(spc.PSW, flag);
}

void tcall(unsigned short addr) {
	push((spc.PC & 0xFF00) >> 8);
	push(spc.PC & 0xFF);
	spc.PC = addr;
}

void tclr1() {
	unsigned char data = spc_readMemory(spc.EA);
	data = data &~ spc.A;

	spc_writeMemory(spc.EA, data);

	if ((spc.A - data)) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if ((spc.A - data) & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void tset1() {
	unsigned char data = spc_readMemory(spc.EA);

	data = data | spc.A;

	spc_writeMemory(spc.EA, data);

	if ((spc.A - data)) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if ((spc.A - data) & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void mul() {
	unsigned short YA = spc.Y * spc.A;

	spc.Y = (YA & 0xFF00) >> 8;
	spc.A = (YA & 0x00FF);

	if (spc.Y) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.Y & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void and1_not() {
	unsigned char data = spc_readMemory(spc.EA & 0x1FFF);

	if (data & (1 << (spc.EA >> 13))) {
		CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	}
}

void and1() {
	unsigned char data = spc_readMemory(spc.EA & 0x1FFF);

	if (!(data & (1 << (spc.EA >> 13)))) {
		CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	}
}

void eor1() {
	unsigned char data = spc_readMemory(spc.EA & 0x1FFF);

	if (data & (1 << (spc.EA >> 13))) {
		spc.PSW = spc.PSW ^ PSW_FLAG_CARRY;
	}
}

void mov1_c() {
	unsigned char data = spc_readMemory(spc.EA & 0x1FFF);

	if (data & (1 << (spc.EA >> 13))) {
		spc.PSW = spc.PSW & ~PSW_FLAG_CARRY;
	} else {
		spc.PSW = spc.PSW | PSW_FLAG_CARRY;
	}
}

void mov1() {
	unsigned char data = spc_readMemory(spc.EA & 0x1FFF);

	if (spc.PSW & PSW_FLAG_CARRY) {
		data = data | (1 << (spc.EA >> 13));
	} else {
		data = data & ~(1 << (spc.EA >> 13));
	}

	spc_writeMemory(spc.EA & 0x1FFF, data);
}

void not1() {
	unsigned char data = spc_readMemory(spc.EA & 0x1FFF);
	spc_writeMemory(spc.EA & 0x1FFF, data ^ (1 << (spc.EA >> 13)));
}

void or1_not() {
	unsigned char data = spc_readMemory(spc.EA & 0x1FFF);

	if (!(data & (1 << (spc.EA >> 13)))) {
		SET_FLAG(spc.PSW, PSW_FLAG_CARRY);
	}
}

void or1() {
	unsigned char data = spc_readMemory(spc.EA & 0x1FFF);

	if (data & (1 << (spc.EA >> 13))) {
		SET_FLAG(spc.PSW, PSW_FLAG_CARRY);
	}
}

void daa() {
	if ((spc.PSW & PSW_FLAG_CARRY) || (spc.A > 0x99)) {
		SET_FLAG(spc.PSW, PSW_FLAG_CARRY);
		spc.A += 0x60;
	}

	if ((spc.PSW & PSW_FLAG_HALFCARRY) || ((spc.A & 0x0F) > 0x9)) {
		spc.A += 0x6;
	}

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}

void das() {
	if ((spc.PSW & PSW_FLAG_CARRY) || (spc.A > 0x99)) {
		SET_FLAG(spc.PSW, PSW_FLAG_CARRY);
		spc.A -= 0x60;
	}

	if ((spc.PSW & PSW_FLAG_HALFCARRY) || ((spc.A & 0xF) > 0x9)) {
		spc.A -= 0x6;
	}

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}

void xcn() {
	spc.A = (spc.A >> 4) | (spc.A << 4);
	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}


/* SPC Init CPU */
void spc_initCPU() {
	/** clear SPC memory and load bootROM */
	memset(spc_mem, 0x0, 0xFFFF);
	memset(&spc, 0x0, sizeof(spc700));
	/* set PC to startup */
	spc.PC = 0xFFC0;
	spc.A = 0;
	spc.X = 0;
	spc.Y = 0;
	spc.PSW = 0x2;
	spc.SP = 0xEF;

	memset(&info, 0x0, sizeof(spc_info));
	info.core = &spc;
}

/** SPC execute cpu */
void spc_run() {
	unsigned char opcode = spc_mem[spc.PC++];

#ifdef DEBUG
	fprintf(stdout, "%04X: A: %02X X: %02X Y: %02X SP: 01%02X PSW: %02X OpCode: %02X EA: %04X\n",
		spc.PC-1, spc.A, spc.X, spc.Y, spc.SP, spc.PSW, opcode, spc.EA);
#endif

	switch (opcode) {
		/* cpu load/store commands */

		// register manipulation
		case 0xE8: // MOV A, #nn
			immediate();
			mov_a();
			spc.cycles += 2;
		break;

		case 0xCD: // MOV X, #nn
			immediate();
			mov_x();
			spc.cycles += 2;
		break;

		case 0x8D: // MOV Y, #nn
			immediate();
			mov_y();
			spc.cycles += 2;
		break;

		case 0x7D: // MOV A,X
			mov_ax();
			spc.cycles += 2;
		break;

		case 0x5D: // MOV X,A
			mov_xa();
			spc.cycles += 2;
		break;

		case 0xDD: // MOV A, Y
			mov_ay();
			spc.cycles += 2;
		break;

		case 0xFD: // MOV Y, A
			mov_ya();
			spc.cycles += 2;
		break;

		case 0x9D: //MOV X, SP
			mov_xsp();
			spc.cycles += 2;
		break;

		case 0xBD: // MOV SP, X
			mov_spx();
			spc.cycles += 2;
		break;

		// memory load

		case 0xE4: // MOV A, [aa]
			directPage();
			mov_a();
			spc.cycles += 3;
		break;

		case 0xF4: // MOV A, [aa+x]
			directPage_indexedX();
			mov_a();
			spc.cycles += 4;
		break;

		case 0xE5: // MOV A, [aaaa]
			absolute();
			mov_a();
			spc.cycles += 4;
		break;

		case 0xF5: // MOV A, [aaaa+x]
			absolute_indexedX();
			mov_a();
			spc.cycles += 5;
		break;

		case 0xF6: // MOV a, [aaaa+y]
			absolute_indexedY();
			mov_a();
			spc.cycles += 5;
		break;

		case 0xE6: // MOV a, [X]
			indirect();
			mov_a();
			spc.cycles += 3;
		break;

		case 0xBF: // MOV A, [X]+
			indirect_autoIncrement();
			mov_a();
			spc.cycles += 4;
		break;

		case 0xF7: // MOV a, [aa]+Y
			indirect_indexedY_indirect();
			mov_a();
			spc.cycles += 6;
		break;

		case 0xE7: //MOV A, [aa+x]
			indirect_indexedX();
			mov_a();
			spc.cycles += 6;
		break;

		case 0xF8: //MOV X, [aa]
			directPage();
			mov_x();
			spc.cycles += 3;
		break;

		case 0xF9: //MOV X, [aa+y]
			directPage_indexedY();
			mov_x();
			spc.cycles += 4;
		break;

		case 0xE9: //MOV X, [aaaa]
			absolute();
			mov_x();
			spc.cycles += 4;
		break;

		case 0xEB: //MOV Y, [aa]
			directPage();
			mov_y();
			spc.cycles += 3;
		break;

		case 0xFB: // MOV Y, [a+y]
			directPage_indexedY();
			mov_y();
			spc.cycles += 4;
		break;

		case 0xEC: // MOV Y, [aaaa]
			absolute();
			mov_y();
			spc.cycles += 4;
		break;

		case 0xBA: // MOVW YA, [aa]
			directPage();
			movw_ya();
			spc.cycles += 5;
		break;


		case 0x8F: // MOV aa, #nn
			immediate();
			mov_dp();
			spc.cycles += 5;
		break;

		case 0xFA: // MOV aa, aa
			directPage();
			mov_dp();
			spc.cycles += 5;
		break;

		case 0xC4: // MOV aa, A
			mov_dp_reg(spc.A);
			spc.cycles += 4;
		break;

		case 0xD8: // MOV aa, X
			mov_dp_reg(spc.X);
			spc.cycles += 4;
		break;

		case 0xCB: // MOV aa, Y
			mov_dp_reg(spc.Y);
			spc.cycles += 4;
		break;

		case 0xD4: // MOV aa+X, A
			mov_dp_x(spc.A);
			spc.cycles += 5;
		break;

		case 0xDB: // MOV aa+X, Y
			mov_dp_x(spc.Y);
			spc.cycles += 5;
		break;

		case 0xD9: // MOV aa+Y, X
			mov_dp_y(spc.X);
			spc.cycles += 5;
		break;

		case 0xC5: // MOV !aaaa, A
			mov_abs(spc.A);
			spc.cycles += 5;
		break;

		case 0xC9: // MOV !aaaa, X
			mov_abs(spc.X);
			spc.cycles += 5;
		break;

		case 0xCC: // MOV !aaaa, Y
			mov_abs(spc.Y);
			spc.cycles += 5;
		break;

		case 0xD5: // MOV !aaaa+X, A
			mov_abs_x(spc.A);
			spc.cycles += 6;
		break;

		case 0xD6: // MOV !aaaa+Y, A
			mov_abs_y(spc.A);
			spc.cycles += 6;
		break;

		case 0xAF: // MOV (X)+, A
			indirect_autoIncrement();
			spc_writeMemory(spc.EA, spc.A);
			spc.cycles += 4;
		break;

		case 0xAA:
			absolute();
			mov1_c();
			spc.cycles += 4;
		break;

		case 0xCA:
			absolute();
			mov1();
			spc.cycles += 6;
		break;

		case 0xC6: // MOV (X), A
			indirect();
			spc_writeMemory(spc.EA, spc.A);
			spc.cycles += 4;
		break;

		case 0xD7: // MOV [aa]+Y, A
			indirect_indexedY_indirect();
			spc_writeMemory(spc.EA, spc.A);
			spc.cycles += 7;
		break;

		case 0xC7: // MOV [aa+X], A
			indirect_indexedX();
			spc_writeMemory(spc.EA, spc.A);
			spc.cycles += 7;
		break;

		case 0xDA: // MOVW aa, YA
			directPage();
			mov_dp_ya();
			spc.cycles += 5;
		break;

		case 0x2D: // PUSH (A)
			push(spc.A);
			spc.cycles += 4;
		break;

		case 0x4D: // PUSH (X)
			push(spc.X);
			spc.cycles += 4;
		break;

		case 0x6D: // PUSH (Y)
			push(spc.Y);
			spc.cycles += 4;
		break;

		case 0x0D: // PUSH (PSW)
			push(spc.PSW);
			spc.cycles += 4;
		break;

		case 0xAE: // POP A
			spc.A = pop();
			spc.cycles += 4;
		break;

		case 0xCE: // POP X
			spc.X = pop();
			spc.cycles += 4;
		break;

		case 0xEE: // POP Y
			spc.Y = pop();
			spc.cycles += 4;
		break;

		case 0x8E: // POP PSW
			spc.PSW = pop();
			spc.cycles += 4;
		break;

		case 0x88: // ADC A, #nn
			immediate();
			adc();
			spc.cycles += 2;
		break;

		case 0x86: // ADC A, [X]
			indirect();
			adc();
			spc.cycles += 3;
		break;

		case 0x84: // ADC A, aa
			directPage();
			adc();
			spc.cycles += 3;
		break;

		case 0x94: // ADC A, aa+x
			directPage_indexedX();
			adc();
			spc.cycles += 4;
		break;

		case 0x85: // ADC A, !aaaa
			absolute();
			adc();
			spc.cycles += 4;
		break;

		case 0x95: // ADC A, !aaaa+X
			absolute_indexedX();
			adc();
			spc.cycles += 5;
		break;

		case 0x96: // ADC A, !aaaa+y
			absolute_indexedY();
			adc();
			spc.cycles += 5;
		break;

		case 0x87: // ADC A, [aa+x]
			indirect_indexedX();
			adc();
			spc.cycles += 6;
		break;

		case 0x97: // ADC A, [aa]+y
			indirect_indexedY_indirect();
			adc();
			spc.cycles += 6;
		break;

		case 0x99: // ADC [x], [y]
			adc_xy();
			spc.cycles += 5;
		break;

		case 0x89: // ADC dp, dp
			directPage();
			adc_dp();
			spc.cycles += 6;
		break;

		case 0x98: // ADC dp, #imm
			immediate();
			adc_imm();
			spc.cycles += 5;
		break;

		case 0xA8: // SBC A, #imm
			immediate();
			sbc();
			spc.cycles += 2;
		break;

		case 0xA6: // SBC A, [X]
			indirect();
			sbc();
			spc.cycles += 3;
		break;

		case 0xA4: // SBC A, aa
			directPage();
			sbc();
			spc.cycles += 3;
		break;

		case 0xB4: // SBC A, aa+x
			directPage_indexedX();
			sbc();
			spc.cycles += 4;
		break;

		case 0xA5: // SBC A, !aaaa
			absolute();
			sbc();
			spc.cycles += 4;
		break;

		case 0xB5: // SBC A, !aaaa+x
			absolute_indexedX();
			sbc();
			spc.cycles += 5;
		break;

		case 0xB6: // SBC A, !aaaa+y
			absolute_indexedY();
			sbc();
			spc.cycles += 5;
		break;

		case 0xA7: // SBC A, [aa+x]
			indirect_indexedX();
			sbc();
			spc.cycles += 6;
		break;

		case 0xB7: // SBC A, [aa]+y
			indirect_indexedY_indirect();
			sbc();
			spc.cycles += 6;
		break;

		case 0xB9: // SBC [X], [Y]
			sbc_xy();
			spc.cycles += 5;
		break;

		case 0xA9: // SBC aa, aa
			directPage();
			sbc_dp();
			spc.cycles += 6;
		break;

		case 0xB8: // SBC aa, #nn
			immediate();
			sbc_imm();
			spc.cycles += 5;
		break;

		case 0x68:
			immediate();
			cmp();
			spc.cycles += 2;
		break;

		case 0x66:
			indirect();
			cmp();
			spc.cycles += 3;
		break;

		case 0x64:
			directPage();
			cmp();
			spc.cycles += 3;
		break;

		case 0x74:
			directPage_indexedX();
			cmp();
			spc.cycles += 4;
		break;

		case 0x65:
			absolute();
			cmp();
			spc.cycles += 4;
		break;

		case 0x75:
			absolute_indexedX();
			cmp();
			spc.cycles += 5;
		break;

		case 0x76:
			absolute_indexedY();
			cmp();
			spc.cycles += 5;
		break;

		case 0x67:
			indirect_indexedX();
			cmp();
			spc.cycles += 6;
		break;

		case 0x77:
			indirect_indexedY_indirect();
			cmp();
			spc.cycles += 6;
		break;

		case 0x79:
			cmp_xy();
			spc.cycles += 5;
		break;

		case 0x69:
			directPage();
			cmp_dp();
			spc.cycles += 6;
		break;

		case 0x78:
			immediate();
			cmp_dp();
			spc.cycles += 5;
		break;

		case 0xC8:
			immediate();
			cpx();
			spc.cycles += 2;
		break;

		case 0x3E:
			directPage();
			cpx();
			spc.cycles += 3;
		break;

		case 0x1E:
			absolute();
			cpx();
			spc.cycles += 4;
		break;

		case 0xAD:
			immediate();
			cpy();
			spc.cycles += 2;
		break;

		case 0x7E:
			directPage();
			cpy();
			spc.cycles += 3;
		break;

		case 0x5E:
			absolute();
			cpy();
			spc.cycles += 4;
		break;

		case 0x28:
			immediate();
			and();
			spc.cycles += 2;
		break;

		case 0x26:
			indirect();
			and();
			spc.cycles += 3;
		break;

		case 0x24:
			directPage();
			and();
			spc.cycles += 3;
		break;

		case 0x34:
			directPage_indexedX();
			and();
			spc.cycles += 4;
		break;

		case 0x25:
			absolute();
			and();
			spc.cycles += 4;
		break;

		case 0x35:
			absolute_indexedX();
			and();
			spc.cycles += 5;
		break;

		case 0x36:
			absolute_indexedY();
			and();
			spc.cycles += 5;
		break;

		case 0x27:
			indirect_indexedX();
			and();
			spc.cycles += 6;
		break;

		case 0x37:
			indirect_indexedY_indirect();
			and();
			spc.cycles += 6;
		break;

		case 0x6A:
			absolute();
			and1_not();
			spc.cycles += 4;
		break;

		case 0x4A:
			absolute();
			and1();
			spc.cycles += 4;
		break;

		case 0x39:
			and_xy();
			spc.cycles += 5;
		break;

		case 0x29:
			directPage();
			and_dp();
			spc.cycles += 5;
		break;

		case 0x38:
			immediate();
			and_dp();
			spc.cycles += 5;
		break;

		case 0x08:
			immediate();
			or();
			spc.cycles += 2;
		break;

		case 0x06:
			indirect();
			or();
			spc.cycles += 3;
		break;

		case 0x04:
			directPage();
			or();
			spc.cycles += 3;
		break;

		case 0x14:
			directPage_indexedX();
			or();
			spc.cycles += 4;
		break;

		case 0x05:
			absolute();
			or();
			spc.cycles += 4;
		break;

		case 0x15:
			absolute_indexedX();
			or();
			spc.cycles += 5;
		break;

		case 0x16:
			absolute_indexedY();
			or();
			spc.cycles += 5;
		break;

		case 0x07:
			indirect_indexedX();
			or();
			spc.cycles += 6;
		break;

		case 0x17:
			indirect_indexedY_indirect();
			or();
			spc.cycles += 6;
		break;

		case 0x19:
			or_xy();
			spc.cycles += 5;
		break;

		case 0x09:
			directPage();
			or_dp();
			spc.cycles += 6;
		break;

		case 0x18:
			immediate();
			or_dp();
			spc.cycles += 5;
		break;

		case 0x48:
			immediate();
			eor();
			spc.cycles += 2;
		break;

		case 0x46:
			indirect();
			eor();
			spc.cycles += 3;
		break;

		case 0x44:
			directPage();
			eor();
			spc.cycles += 3;
		break;

		case 0x54:
			directPage_indexedX();
			eor();
			spc.cycles += 4;
		break;

		case 0x45:
			absolute();
			eor();
			spc.cycles += 4;
		break;

		case 0x55:
			absolute_indexedX();
			eor();
			spc.cycles += 5;
		break;

		case 0x56:
			absolute_indexedY();
			eor();
			spc.cycles += 5;
		break;

		case 0x47:
			indirect_indexedX();
			eor();
			spc.cycles += 6;
		break;

		case 0x57:
			indirect_indexedY_indirect();
			eor();
			spc.cycles += 6;
		break;

		case 0x59:
			eor_xy();
			spc.cycles += 5;
		break;

		case 0x49:
			directPage();
			eor_dp();
			spc.cycles += 6;
		break;

		case 0x58:
			immediate();
			eor_dp();
			spc.cycles += 5;
		break;

		case 0x8A:
			absolute();
			eor1();
			spc.cycles += 5;
		break;

		case 0xBC:
			inc_reg(&spc.A);
			spc.cycles += 2;
		break;

		case 0xAB:
			directPage();
			inc_mem();
			spc.cycles += 4;
		break;

		case 0xBB:
			directPage_indexedX();
			inc_mem();
			spc.cycles += 5;
		break;

		case 0xAC:
			absolute();
			inc_mem();
			spc.cycles += 5;
		break;

		case 0x3D:
			inc_reg(&spc.X);
			spc.cycles += 2;
		break;

		case 0xFC:
			inc_reg(&spc.Y);
			spc.cycles += 2;
		break;

		case 0x9C:
			dec_reg(&spc.A);
			spc.cycles += 2;
		break;

		case 0x8B:
			directPage();
			dec_mem();
			spc.cycles += 4;
		break;

		case 0x9B:
			directPage_indexedX();
			dec_mem();
			spc.cycles += 5;
		break;

		case 0x8C:
			absolute();
			dec_mem();
			spc.cycles += 5;
		break;

		case 0x1D:
			dec_reg(&spc.X);
			spc.cycles += 2;
		break;

		case 0xDC:
			dec_reg(&spc.Y);
			spc.cycles += 2;
		break;

		/* ASL */
		case 0x1C:
			asl_a();
			spc.cycles += 2;
		break;

		case 0x0B:
			directPage();
			asl_mem();
			spc.cycles += 4;
		break;

		case 0x1B:
			directPage_indexedX();
			asl_mem();
			spc.cycles += 5;
		break;

		case 0x0C:
			absolute();
			asl_mem();
			spc.cycles += 5;
		break;

		/* LSR */
		case 0x5C:
			lsr_a();
			spc.cycles += 2;
		break;

		case 0x4B:
			directPage();
			lsr_mem();
			spc.cycles += 4;
		break;

		case 0x5B:
			directPage_indexedX();
			lsr_mem();
			spc.cycles += 5;
		break;

		case 0x4C:
			absolute();
			lsr_mem();
			spc.cycles += 5;
		break;

		/* ROL */
		case 0x3C:
			rol_a();
			spc.cycles += 2;
		break;

		case 0x2B:
			directPage();
			rol_mem();
			spc.cycles += 4;
		break;

		case 0x3B:
			directPage_indexedX();
			rol_mem();
			spc.cycles += 4;
		break;

		case 0x2C:
			absolute();
			rol_mem();
			spc.cycles += 4;
		break;

		/* ROR */
		case 0x7C:
			ror_a();
			spc.cycles += 2;
		break;

		case 0x6B:
			directPage();
			ror_mem();
			spc.cycles += 4;
		break;

		case 0x7B:
			directPage_indexedX();
			ror_mem();
			spc.cycles += 4;
		break;

		case 0x6C:
			absolute();
			ror_mem();
			spc.cycles += 5;
		break;

		/* XCN */
		case 0x9F:
			xcn();
			spc.cycles += 5;
		break;

		/* 16 bit operation commands */
		case 0x3A:
			directPage();
			incw();
			spc.cycles += 6;
		break;

		case 0x1A:
			directPage();
			decw();
			spc.cycles += 6;
		break;

		case 0x7A:
			directPage();
			addw();
			spc.cycles += 5;
		break;

		case 0x9A:
			directPage();
			subw();
			spc.cycles += 5;
		break;

		case 0x5A:
			directPage();
			cmpw();
			spc.cycles += 4;
		break;

		case 0x13:
			directPage();
			bbc(0x1);
			spc.cycles += 5;
		break;

		case 0x33:
			directPage();
			bbc(0x2);
			spc.cycles += 5;
		break;

		case 0x53:
			directPage();
			bbc(0x4);
			spc.cycles += 5;
		break;

		case 0x73:
			directPage();
			bbc(0x8);
			spc.cycles += 5;
		break;

		case 0x93:
			directPage();
			bbc(0x10);
			spc.cycles += 5;
		break;

		case 0xB3:
			directPage();
			bbc(0x20);
			spc.cycles += 5;
		break;

		case 0xD3:
			directPage();
			bbc(0x40);
			spc.cycles += 5;
		break;

		case 0xF3:
			directPage();
			bbc(0x80);
			spc.cycles += 5;
		break;

		case 0x03:
			directPage();
			bbs(0x1);
			spc.cycles += 5;
		break;

		case 0x23:
			directPage();
			bbs(0x2);
			spc.cycles += 5;
		break;

		case 0x43:
			directPage();
			bbs(0x4);
			spc.cycles += 5;
		break;

		case 0x63:
			directPage();
			bbs(0x8);
			spc.cycles += 5;
		break;

		case 0x83:
			directPage();
			bbs(0x10);
			spc.cycles += 5;
		break;

		case 0xA3:
			directPage();
			bbs(0x20);
			spc.cycles += 5;
		break;

		case 0xC3:
			directPage();
			bbs(0x40);
			spc.cycles += 5;
		break;

		case 0xE3:
			directPage();
			bbs(0x80);
			spc.cycles += 5;
		break;

		case 0x90:
			bcc();
			spc.cycles += 2;
		break;

		case 0xB0:
			bcs();
			spc.cycles += 2;
		break;

		case 0xF0:
			beq();
			spc.cycles += 2;
		break;

		case 0x30:
			bmi();
			spc.cycles += 2;
		break;

		case 0xD0:
			bne();
			spc.cycles += 2;
		break;

		case 0x10:
			bpl();
			spc.cycles += 2;
		break;

		case 0x50:
			bvc();
			spc.cycles += 2;
		break;

		case 0x70:
			bvs();
			spc.cycles += 2;
		break;

		case 0x2F:
			bra();
			spc.cycles += 4;
		break;

		case 0x0F:
			brk();
			spc.cycles += 8;
		break;

		case 0x3F:
		  	absolute();
			call();
			spc.cycles += 8;
		break;

		case 0xDE:
			directPage_indexedX();
			cbne();
			spc.cycles += 6;
		break;

		case 0x2E:
			directPage();
			cbne();
			spc.cycles += 5;
		break;

		case 0x12:
			directPage();
			clr1(0x1);
			spc.cycles += 4;
		break;

		case 0x32:
			directPage();
			clr1(0x2);
			spc.cycles += 4;
		break;

		case 0x52:
			directPage();
			clr1(0x4);
			spc.cycles += 4;
		break;

		case 0x72:
			directPage();
			clr1(0x8);
			spc.cycles += 4;
		break;

		case 0x92:
			directPage();
			clr1(0x10);
			spc.cycles += 4;
		break;

		case 0xB2:
			directPage();
			clr1(0x20);
			spc.cycles += 4;
		break;

		case 0xD2:
			directPage();
			clr1(0x40);
			spc.cycles += 4;
		break;

		case 0xF2:
			directPage();
			clr1(0x80);
			spc.cycles += 4;
		break;

		case 0x60:
			clrc();
			spc.cycles += 2;
		break;

		case 0x20:
			clrp();
			spc.cycles += 2;
		break;

		case 0xE0:
			clrv();
			spc.cycles += 2;
		break;

		case 0xC0:
			di();
			spc.cycles += 3;
		break;

		case 0xA0:
			ei();
			spc.cycles += 3;
		break;

		case 0x9E:
			DIV();
			spc.cycles += 12;
		break;

		case 0x1F:
			absolute_indexedX();
			jmp_1f();
			spc.cycles += 6;
		break;

		case 0x5F:
			absolute();
			jmp_5f();
			spc.cycles += 3;
		break;

		case 0x00:
			// NOP
			spc.cycles += 2;
		break;

		case 0xED:
			notc();
			spc.cycles += 3;
		break;

		case 0x6F:
			ret();
			spc.cycles += 5;
		break;

		case 0x7F:
			ret1();
			spc.cycles += 6;
		break;

		case 0x02:
			directPage();
			set1(0x1);
			spc.cycles += 4;
		break;

		case 0x22:
			directPage();
			set1(0x2);
			spc.cycles += 4;
		break;

		case 0x42:
			directPage();
			set1(0x4);
			spc.cycles += 4;
		break;

		case 0x62:
			directPage();
			set1(0x8);
			spc.cycles += 4;
		break;

		case 0x82:
			directPage();
			set1(0x10);
			spc.cycles += 4;
		break;

		case 0xA2:
			directPage();
			set1(0x20);
			spc.cycles += 4;
		break;

		case 0xC2:
			directPage();
			set1(0x40);
			spc.cycles += 4;
		break;

		case 0xE2:
			directPage();
			set1(0x80);
			spc.cycles += 4;
		break;

		case 0x80:
			setPSW(PSW_FLAG_CARRY);
			spc.cycles += 2;
		break;

		case 0x40:
		    setPSW(PSW_FLAG_DIRECTPAGE);
			spc.cycles += 2;
		break;

		case 0xFF:
			printf("SLEEP\n");
			exit(0);

			spc.cycles += 1;
		break;

		case 0xEF:
			printf("STOP\n");
			spc.cycles += 1;
		break;

		case 0x01:
			tcall(0xFFDE);
			spc.cycles += 8;
		break;

		case 0x11:
			tcall(0xFFDC);
			spc.cycles += 8;
		break;

		case 0x21:
			tcall(0xFFDA);
			spc.cycles += 8;
		break;

		case 0x31:
			tcall(0xFFD8);
			spc.cycles += 8;
		break;

		case 0x41:
			tcall(0xFFD6);
			spc.cycles += 8;
		break;

		case 0x51:
			tcall(0xFFD4);
			spc.cycles += 8;
		break;

		case 0x61:
			tcall(0xFFD2);
			spc.cycles += 8;
		break;

		case 0x71:
			tcall(0xFFD0);
			spc.cycles += 8;
		break;

		case 0x81:
			tcall(0xFFCE);
			spc.cycles += 8;
		break;

		case 0x91:
			tcall(0xFFCC);
			spc.cycles += 8;
		break;


		case 0xA1:
			tcall(0xFFCA);
			spc.cycles += 8;
		break;

		case 0xB1:
			tcall(0xFFC8);
			spc.cycles += 8;
		break;

		case 0xC1:
			tcall(0xFFC6);
			spc.cycles += 8;
		break;

		case 0xD1:
			tcall(0xFFC4);
			spc.cycles += 8;
		break;

		case 0xE1:
			tcall(0xFFC2);
			spc.cycles += 8;
		break;

		case 0xF1:
			tcall(0xFFC0);
			spc.cycles += 8;
		break;

		case 0x4E:
		    absolute();
			tclr1();
			spc.cycles += 6;
		break;

		case 0x0E:
			absolute();
			tset1();
			spc.cycles += 6;
		break;

		case 0x4F:
			tcall(0xFF00 | spc_readMemory(spc.PC++));
			spc.cycles += 6;
		break;

		case 0xCF:
			mul();
			spc.cycles += 9;
		break;

		case 0xFE:
			dbnz_y();
			spc.cycles += 4;
		break;

		case 0x6E:
			dbnz_d();
			spc.cycles += 4;
		break;

		case 0xEA:
			absolute();
			not1();
			spc.cycles += 5;
		break;

		case 0x2A:
			absolute();
			or1_not();
			spc.cycles += 5;
		break;

		case 0x0A:
			absolute();
			or1();
			spc.cycles += 5;
		break;

		case 0xDF:
			daa();
			spc.cycles += 3;
		break;

		case 0xBE:
			das();
			spc.cycles += 3;
		break;

		default:
			printf("Opcode: 0x%X not implemented\n", opcode);
		break;
	}
}

void spc_updateTimers() {
	for (int i = 0; i < 3; i++) {
		spc.tmrList[i].cycles += spc.cycles;

		if (spc.tmrList[i].cycles > (i > 1 ? 16 : 128)) {
			if (spc.tmrList[i].enable) {
					spc.tmrList[i].inCnt++;

					if (spc.tmrList[i].inCnt >= spc.tmrList[i].tmr) {
						spc.tmrList[i].cnt = (spc.tmrList[i].cnt + 1) & 0xF;
						spc.tmrList[i].inCnt = 0;
					}

			}
			spc.tmrList[i].cycles = spc.tmrList[i].cycles - (i > 1 ? 16 : 128);
		}
	}
}
