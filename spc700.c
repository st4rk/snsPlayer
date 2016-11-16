#include "spc700.h"

/* SPU */

spc_700 spc;

/* Internal Memory 64 KB */
unsigned char spc_mem[0xFA00];

/* Write 8 bits to memory */
void spc_writeMemory(unsigned short addr, unsigned char data) {
	printf("Debug - Write Addr: 0x%X   - Data: 0x%X\n", addr, data);

	addr = addr & 0xFFFF;

	/* PAGE 0 */
	if ((addr >= 0x0000) && (addr <= 0x00EF)) {

	}

	/* Registers */
	if ((addr >= 0x00F0) && (addr <= 0x00FF)) {
		switch (addr & 0xFF) {
			case 0xF0:

			break;

			/* Control Register */
			case 0xF1:

			break;

			case 0xF2:

			break;

			case 0xF3:

			break;

			case 0xF4:

			break;

			case 0xF5:

			break;

			case 0xF6:

			break;

			case 0xF7:

			break;

			case 0xF8:

			break;

			case 0xF9:

			break;

			case 0xFA:

			break;

			case 0xFB:

			break;

			case 0xFC:

			break;

			case 0xFD:

			break;
			
			case 0xFE:

			break;

			case 0xFF:

			break;

			default:
				printf("Register not found, register : 0x%X data: 0x%X\n",(addr & 0xFF), data);
			break;
		}
	}

	/* Page 1 */
	if ((addr >= 0x0100) && (addr <= 0x01FF)) {

	}

	/* Memory */
	if ((addr >= 0x0200) && (addr <= 0xFFBF)) {

	}

	/* Memory (read/write) */
	if ((addr >= 0xFFC0) && (addr <= 0xFFFF)) {

	}

}

/* Read 8 bits from memory */
unsigned char spc_readMemory(unsigned short addr) {
	printf("Debug - Read Addr: 0x%X\n", addr);

	addr = addr & 0xFFFF;

	/* PAGE 0 */
	if ((addr >= 0x0000) && (addr <= 0x00EF)) {

	}

	/* Registers */
	if ((addr >= 0x00F0) && (addr <= 0x00FF)) {

	}

	/* Page 1 */
	if ((addr >= 0x0100) && (addr <= 0x01FF)) {

	}

	/* Memory */
	if ((addr >= 0x0200) && (addr <= 0xFFBF)) {

	}

	/* Memory (read/write) */
	if ((addr >= 0xFFC0) && (addr <= 0xFFFF)) {

	}

}

/* Stack Manipulation */
void push(unsigned char data) {
	spc_writeMemory((spc.SP + 0x0100), data);

	spc.SP = ((spc.SP - 1) & 0xFF);
}

unsigned char pop() {
	spc.SP = ((spc.SP + 1) & 0xFF);

	return (spc_readMemory((spc.SP + 0x0100)));
}

/* Addressing mode */

void immediate() {
	spc.EA = spc.PC++;
}

void directPage() {
	spc.EA = spc_readMemory(spc.PC++) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3);
}

void directPage_indexedX() {
	spc.EA = (spc_readMemory(spc.PC++) + spc.X) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3);
}

void directPage_indexedY()  {
	spc.EA = (spc_readMemory(spc.PC++) + spc.Y) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3);
}

void indirect() {
	spc.EA = (spc_readMemory(spc.X) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
}

void indirect_autoIncrement() {
	spc.EA = (spc_readMemory(spc.X++) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
}

void directPage_directPage() {

}

void indirectPage_indirectPage() {

}

void immData_directPage() {

}

void directPage_bit() {

}

void directPage_bitRelative() {

}

void absolute_booleanBit() {

}

void absolute() {
	EA = (spc_readMemory(spc.PC) | (spc_readMemory(spc.PC + 1) << 8));
	spc.PC += 2;
}

void absolute_indexedX() {
	EA = (spc_readMemory(spc.PC) | (spc_readMemory(spc.PC + 1) << 8)) + spc.Y;
	spc.PC += 2;
}

void absolute_indexedY() {
	EA = (spc_readMemory(spc.PC) | (spc_readMemory(spc.PC + 1) << 8)) + spc.Y;
	spc.PC += 2;
}

void indirect_indexedX() {
	spc.EA = ((spc_readMemory(spc.PC) + spc.X) & 0xFF)| ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3) ;
	spc.EA = spc_readMemory(spc.EA);
	spc.PC++;
}

void indirect_indexedY_indirect() {
	spc.EA = spc_readMemory(spc.PC++) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3);
	spc.EA = spc_readMemory(spc.EA) + spc.Y;
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
	spc.A = spc.Y
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
	A = spc_readMemory(EA);
	Y = spc_readMemory(EA + 1);

	if (A+Y) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (Y & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void mov_dp() {
	unsigned char data =  spc_readMemory(EA);

	directPage();
	spc_writeMemory(EA, data);
}

void mov_dp_reg(unsigned char reg) {

	directPage();
	spc_writeMemory(EA, reg);
}

void mov_dp_x(unsigned char reg) {
	directPage_indexedX();
	spc_writeMemory(EA, reg);
}


void mov_dp_y(unsigned char reg) {
	directPage_indexedY();
	spc_writeMemory(EA, reg);
}

void mov_abs(unsigned char reg) {
	absolute();
	spc_writeMemory(EA, reg);
}

void mov_abs_x(unsigned char reg) {
	absolute_indexedX();
	spc_writeMemory(EA, reg);
}

void mov_abs_y(unsigned char reg) {
	absolute_indexedY();
	spc_writeMemory(EA, reg);
}

void mov_dp_ya() {
	unsigned short data = ((spc.Y << 8) & 0xFF) | spc.A;
	spc_writeMemory(EA);
}

void or() {
	EA = spc_readMemory(EA);

	A = (A | (EA & 0xFF));
	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}


void or_xy() {
	unsigned short X = (spc_readMemory(spc.X) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
	unsigned short Y = (spc_readMemory(spc.Y) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));

	unsigned char X_2 = spc_readMemory(X);
	Y = spc_readMemory(Y);

	X_2 = (X_2 | Y);

	spc_writeMemory(X, X_2);


	if (X_2) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (X_2 & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}

void or_dp() {
	unsigned char data_1 = spc_readMemory(EA);
	directPage();
	unsigned char data_2 = spc_readMemory(EA);

	data_2 = (data_2 | data_1);

	spc_writeMemory(EA, data_2);

	if (data_2) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data_2 & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}

void and() {
	EA = spc_readMemory(EA);

	A = (A & (EA & 0xFF));

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}

void and_xy() {
	unsigned short X = (spc_readMemory(spc.X) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
	unsigned short Y = (spc_readMemory(spc.Y) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));

	unsigned char X_2 = spc_readMemory(X);
	Y = spc_readMemory(Y);

	X_2 = (X_2 & Y);

	spc_writeMemory(X, X_2);


	if (X_2) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (X_2 & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}

void and_dp() {
	unsigned char data_1 = spc_readMemory(EA);
	directPage();
	unsigned char data_2 = spc_readMemory(EA);

	data_2 = (data_2 & data_1);

	spc_writeMemory(EA, data_2);

	if (data_2) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data_2 & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}

void eor() {
	EA = spc_readMemory(EA);

	A = (A ^ (EA & 0xFF));

	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}


void eor_xy() {
	unsigned short X = (spc_readMemory(spc.X) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
	unsigned short Y = (spc_readMemory(spc.Y) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));

	unsigned char X_2 = spc_readMemory(X);
	Y = spc_readMemory(Y);

	X_2 = (X_2 ^ Y);

	spc_writeMemory(X, X_2);


	if (X_2) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (X_2 & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}

void eor_dp() {
	unsigned char data_1 = spc_readMemory(EA);
	directPage();
	unsigned char data_2 = spc_readMemory(EA);

	data_2 = (data_2 ^ data_1);

	spc_writeMemory(EA, data_2);

	if (data_2) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data_2 & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}


void adc() {
	EA = spc_readMemory(EA);
	unsigned short temp = ((EA + spc.A + (spc.PSW & PSW_FLAG_CARRY)) & 0xFF);

	/* Verify half flag */
	if ((spc.A & 0x8)) {
		if ((temp & 0x10) && (!(temp & 0x8)))
			SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
		else
			CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
	}
	/* Verify if a carry happened */
	if (temp > 0xFF) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* Check a overflow */
	if ((~(spc.A ^ EA)) & (spc.A ^ temp) & PSW_FLAG_OVERFLOW) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);
	spc.A = (temp & 0xFF);


	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void adc_imm() {
	unsigned char data_1 = spc_readMemory(EA);
	directPage();
	unsigned char data_2 = spc_readMemory(EA);

	unsigned short temp = ((data_1 + data_2 + (spc.PSW & PSW_FLAG_CARRY)) & 0xFF);

	/* Verify half flag */
	if ((data_2 & 0x8)) {
		if ((temp & 0x10) && (!(temp & 0x8)))
			SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
		else
			CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
	}
	/* Verify if a carry happened */
	if (temp > 0xFF) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* Check a overflow */
	if ((~(data_2 ^ data_1)) & (data_2 ^ temp) & PSW_FLAG_OVERFLOW) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);

	spc_writeMemory(EA, (temp & 0xFF));

	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);



}


void adc_xy() {
	unsigned short X = (spc_readMemory(spc.X) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
	unsigned short Y = (spc_readMemory(spc.Y) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));

	unsigned char X_2 = spc_readMemory(X);
	Y = spc_readMemory(Y);

	unsigned short temp = ((X_2 + Y + (spc.PSW & PSW_FLAG_CARRY)) & 0xFF);

	/* Verify half flag */
	if ((spc.A & 0x8)) {
		if ((temp & 0x10) && (!(temp & 0x8)))
			SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
		else
			CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
	}
	/* Verify if a carry happened */
	if (temp > 0xFF) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* Check a overflow */
	if ((~(X_2 ^ Y)) & (X_2 ^ temp) & PSW_FLAG_OVERFLOW) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);

	spc_writeMemory(X, (temp & 0xFF));

	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);


}

void adc_dp() {
	unsigned char data_1 = spc_readMemory(EA);
	directPage();
	unsigned char data_2 = spc_readMemory(EA);

	unsigned short temp = ((data_1 + data_2 + (spc.PSW & PSW_FLAG_CARRY)) & 0xFF);

	/* Verify half flag */
	if ((data_2 & 0x8)) {
		if ((temp & 0x10) && (!(temp & 0x8)))
			SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
		else
			CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
	}
	/* Verify if a carry happened */
	if (temp > 0xFF) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* Check a overflow */
	if ((~(data_2 ^ data_1)) & (data_2 ^ temp) & PSW_FLAG_OVERFLOW) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);

	spc_writeMemory(EA, (temp & 0xFF));

	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);


}


void sbc() {
	EA = spc_readMemory(EA) ^ 0xFF;
	unsigned short temp = ((EA + spc.A + (spc.PSW & PSW_FLAG_CARRY)) & 0xFF);

	/* Verify half flag */
	if ((spc.A & 0x8)) {
		if ((temp & 0x10) && (!(temp & 0x8)))
			SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
		else
			CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
	}
	/* Verify if a carry happened */
	if (temp > 0xFF) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* Check a overflow */
	if ((~(spc.A ^ EA)) & (spc.A ^ temp) & PSW_FLAG_OVERFLOW) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);
	spc.A = (temp & 0xFF);


	if (spc.A) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (spc.A & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}

void sbc_imm() {
	unsigned char data_1 = spc_readMemory(EA) ^ 0xFF;
	directPage();
	unsigned char data_2 = spc_readMemory(EA);

	unsigned short temp = ((data_1 + data_2 + (spc.PSW & PSW_FLAG_CARRY)) & 0xFF);

	/* Verify half flag */
	if ((data_2 & 0x8)) {
		if ((temp & 0x10) && (!(temp & 0x8)))
			SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
		else
			CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
	}
	/* Verify if a carry happened */
	if (temp > 0xFF) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* Check a overflow */
	if ((~(data_2 ^ data_1)) & (data_2 ^ temp) & PSW_FLAG_OVERFLOW) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);

	spc_writeMemory(EA, (temp & 0xFF));

	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);



}


void sbc_xy() {
	unsigned short X = (spc_readMemory(spc.X) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
	unsigned short Y = (spc_readMemory(spc.Y) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));

	unsigned char X_2 = spc_readMemory(X) ^ 0xFF;
	Y = spc_readMemory(Y);

	unsigned short temp = ((X_2 + Y + (spc.PSW & PSW_FLAG_CARRY)) & 0xFF);

	/* Verify half flag */
	if ((spc.A & 0x8)) {
		if ((temp & 0x10) && (!(temp & 0x8)))
			SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
		else
			CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
	}
	/* Verify if a carry happened */
	if (temp > 0xFF) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* Check a overflow */
	if ((~(X_2 ^ Y)) & (X_2 ^ temp) & PSW_FLAG_OVERFLOW) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);

	spc_writeMemory(X, (temp & 0xFF));

	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);


}

void sbc_dp() {
	unsigned char data_1 = spc_readMemory(EA) ^ 0xFF;
	directPage();
	unsigned char data_2 = spc_readMemory(EA);

	unsigned short temp = ((data_1 + data_2 + (spc.PSW & PSW_FLAG_CARRY)) & 0xFF);

	/* Verify half flag */
	if ((data_2 & 0x8)) {
		if ((temp & 0x10) && (!(temp & 0x8)))
			SET_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
		else
			CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
	}
	/* Verify if a carry happened */
	if (temp > 0xFF) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);

	/* Check a overflow */
	if ((~(data_2 ^ data_1)) & (data_2 ^ temp) & PSW_FLAG_OVERFLOW) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);

	spc_writeMemory(EA, (temp & 0xFF));

	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);


}

void cmp() {
	EA = spc_readMemory(EA);

	int temp = spc.A - EA;

	if (!(temp & 0x8000)) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG()
	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void cmp_xy() {
	unsigned short X = (spc_readMemory(spc.X) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
	unsigned short Y = (spc_readMemory(spc.Y) | ((spc.PSW & PSW_FLAG_DIRECTPAGE) << 3));
	X = spc_readMemory(X);
	Y = spc_readMemory(Y);

	int temp = X - Y;

	if (!(temp & 0x8000)) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG()
	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}

void cmp_dp() {
	unsigned char data = spc_readMemory(EA);
	directPage();
	EA = spc_readMemory(EA);

	int temp = EA - data;

	if (!(temp & 0x8000)) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG()
	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void cpx() {
	EA = spc_readMemory(EA);

	int temp = spc.X - EA;

	if (!(temp & 0x8000)) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG()
	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}

void cpy() {
	EA = spc_readMemory(EA);

	int temp = spc.Y - EA;

	if (!(temp & 0x8000)) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG()
	if (temp) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (temp & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);

}

void inc_mem() {
	unsigned char data = spc_readMemory(EA);

	spc_writeMemory(EA, data++);
	if (data) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void inc_reg(unsigned char *reg) {

	*reg++;

	if (*reg) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (*reg & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void dec_mem() {
	unsigned char data = spc_readMemory(EA);

	spc_writeMemory(EA, data--);
	if (data) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void dec_reg(unsigned char *reg) {
	*reg--;

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
	unsigned char data = spc_readMemory(EA);

	if (data & 0x80) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	data <<= 1;

	spc_writeMemory(EA, data);

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
	unsigned char data = spc_readMemory(EA);

	if (data & 0x1) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	data >>= 1;

	spc_writeMemory(EA, data);

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
	unsigned char data = spc_readMemory(EA);
	unsigned char c = spc.PSW & PSW_FLAG_CARRY;

	if (data & 0x80) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	data = (data << 1) | c;

	spc_writeMemory(EA, data);

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
	unsigned char data = spc_readMemory(EA);
	unsigned char c = spc.PSW & PSW_FLAG_CARRY;

	if (data & 0x1) SET_FLAG(spc.PSW, PSW_FLAG_CARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_CARRY);
	data = (data >> 1) | c;

	spc_writeMemory(EA, data);

	if (data) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data & PSW_FLAG_NEGATIVE) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void incw() {
	unsigned short data = (spc_readMemory(EA) | (spc_readMemory(EA + 1) << 8);
	
	data++;

	spc_writeMemory(EA, data & 0xFF);
	spc_writeMemory(EA+1, (data >> 8) & 0xFF);

	if (data) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void decw() {
	unsigned short data = (spc_readMemory(EA) | (spc_readMemory(EA + 1) << 8);
	
	data--;

	spc_writeMemory(EA, data & 0xFF);
	spc_writeMemory(EA+1, (data >> 8) & 0xFF);
	
	if (data) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (data & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void addw() {
	unsigned short data = (spc_readMemory(EA) | (spc_readMemory(EA + 1) << 8);
	unsigned short YA   = A | (Y << 8);
	unsigned int result = YA + data;
	
	if ((~(YA ^ data)) & (YA ^ result) & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);
	if ((YA ^ data ^ result) & 0x1000) SET_FLAG (spc.PSW, PSW_FLAG_HALFCARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
	if (result > 0xFFFF) 

	spc.Y = (YA >> 0x8) & 0xFF;
	spc.A = (YA & 0xFF);

	if (YA) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (YA & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void subw() {
	unsigned short data = (spc_readMemory(EA) | (spc_readMemory(EA + 1) << 8);
	unsigned short YA   = A | (Y << 8);
	unsigned int result = YA - data;
	
	if ((~(YA ^ data)) & (YA ^ result) & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_OVERFLOW); else CLEAR_FLAG(spc.PSW, PSW_FLAG_OVERFLOW);
	if ((YA ^ data ^ result) & 0x1000) SET_FLAG (spc.PSW, PSW_FLAG_HALFCARRY); else CLEAR_FLAG(spc.PSW, PSW_FLAG_HALFCARRY);
	if (result > 0xFFFF) 

	spc.Y = (YA >> 0x8) & 0xFF;
	spc.A = (YA & 0xFF);

	if (YA) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (YA & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

void cmpw() {
	unsigned short data = (spc_readMemory(EA) | (spc_readMemory(EA + 1) << 8);
	unsigned short YA   = A | (Y << 8);

	YA = YA + data;

	if (YA) CLEAR_FLAG(spc.PSW, PSW_FLAG_ZERO); else SET_FLAG(spc.PSW, PSW_FLAG_ZERO);
	if (YA & 0x8000) SET_FLAG(spc.PSW, PSW_FLAG_NEGATIVE); else CLEAR_FLAG(spc.PSW, PSW_FLAG_NEGATIVE);
}

/* SPC Init CPU */
void spc_initCPU() {


	/* Init the Stack 0x0100 ~ 0x01FF*/
	SP = 0xFF;
}

/* SPC Main Loop */
void spc_mainLoop() {
	unsigned char opcode = spc_mem[spc.pc++];

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

		case 0xF6 // MOV a, [aaaa+y]
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

		case 0xFA; // MOV aa, aa
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
			spc_writeMemory(EA, spc.A);
			spc.cycles += 4;
		break;

		case 0xC6: // MOV (X), A
			indirect();
			spc_writeMemory(EA, spc.A);
			spc.cycles += 4;
		break;

		case 0xD7: // MOV [aa]+Y, A
			indirect_indexedY_indirect();
			spc_writeMemory(EA, spc.A);
			spc.cycles += 7;
		break;

		case 0xC7: // MOV [aa+X], A
			indirect_indexedX();
			spc_writeMemory(EA, spc.A);
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
			push(PSW);
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
			sbc();
			spc.cycles += 6;
		break;
		

		case 0x77:
			indirect_indexedY_indirect();
			sbc();
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

		case 0x27;
			indirect_indexedX();
			and();
			spc.cycles += 6;
		break;

		case 0x37:
			indirect_indexedY_indirect();
			and();
			spc.cycles += 6;
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
			dec_reg(&reg.Y);
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

		case 0xCC:
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
			spc.A = (spc.A >> 4) | (spc.A << 4);
			spc.cycles += 5;
		break;

		/* 16 bit operation commands */
		case 0x3A;
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

		default:
			printf("Opcode: 0x%X not implemented\n", opcode);
		break;
	}
}

