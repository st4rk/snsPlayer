/*
 * 6502 CPU 
 * Written by St4rk
 * Illegal opcodes aren't implemented yet
 */


#include "6502.h"

/* system memory */
unsigned char memory[0xFFFFF];

/* system registers */
unsigned char A;
unsigned char P;
unsigned char X;
unsigned char Y;
unsigned char S;
int PC;
int EA;
/* system tick count */
int tick_count;

/* Bank Switching enable */
unsigned char isBank;


/* Write/Read memory */
unsigned char memoryRead(unsigned short addr) {
	/* Verify if it's reading from the APU registers */
	if ((addr >= 0x4000) && (addr <= 0x4017)) {
		if (addr == STATUS_REGISTER) {
			/* TODO: if lenght counter is == 0, don't return the channel flags */
			
			if (squareList[0].len_cnt > 0)
				apu.pulse_channel_1 = 1;
			else
				apu.pulse_channel_1 = 0;

			if (squareList[1].len_cnt > 0)
				apu.pulse_channel_2 = 1;
			else
				apu.pulse_channel_2 = 0;

			if (triangle.len_cnt > 0) {
				apu.triangle_flag = 1;
			} else {
				apu.triangle_flag = 0;
			}


			
			return (apu.dmc_flag | apu.noise_flag | apu.triangle_flag | apu.pulse_channel_2 | apu.pulse_channel_1);
		}
	}

	/* Verify if bank switching is used, if yes, switch the banks */
	if (isBank) {
		if (addr >= 0x8000) {
			unsigned char bankNum = memory[0x5ff8 + ((addr >> 12) - 8)];
			return (memory[0x8000 + (bankNum * 0x1000) + (addr & 0x0FFF)]);
		}
	}

	return (memory[addr]);
}

void writeMemory(unsigned short addr, unsigned char data) {
	if ((addr >= 0x4000) && (addr <= 0x4017)) {

		if ((addr >= 0x4000) && (addr <= 0x4003)) {
			switch (addr) {
				case PULSE1_DUTY_ENV:
					/* Set Duty */
					squareList[0].duty              = ((data & 0xC0) >> 0x6);
					/* Set haltFlag/envelope loop flag */
					squareList[0].env.loop_flag     = ((data & 0x20) >> 0x5);
					/* Set const flag */
					squareList[0].env.c_flag        = ((data & 0x10) >> 0x4);
					/* Set Volume */
					squareList[0].env.volume        = (data & 0xF);
					/* Load the envelope counter */
					squareList[0].env.down_cnt      = (data & 0xF) + 1;
				break;

				case PULSE1_SWEEP:
					/* Store all sweep information */
					/* write on this register result in reload the divider with period */
					squareList[0].swp.enable        = ((data & 0x80) >> 0x7);
					squareList[0].swp.period        = ((data & 0x70) >> 0x4);
					squareList[0].swp.neg_flag      = ((data & 0x08) >> 0x3);
					squareList[0].swp.shift_cnt     =  (data & 0x07);
					squareList[0].swp.divider_cnt   = squareList[0].swp.period;
				break;

				/* Set the low 8 bits on timer */
				case PULSE1_PERIOD_LOW:
					squareList[0].timer = ((squareList[0].timer & 0x700) | data);
					square_freq_output(SQUARE_WAVE_UNIT_1);
				break;

				/* load the high 3 bits of timer */
				case PULSE1_LEN_PE_HIGH:
					squareList[0].timer          = ((squareList[0].timer & 0xFF) | ((data & 0x7) << 8));
					/* It reset the envelope decay counter to 0xF(max) state */
					squareList[0].env.down_cnt   = 0xF;
					/* Load the lenght counter( find the proper value on table ) */
					squareList[0].len_cnt        = getLengthCnt(((data & 0xF8) >> 3)); 
					square_freq_output(SQUARE_WAVE_UNIT_1);
				break;

				default:	
					printf("Register 0x%X not implemented \n", addr);
				break;
			}
		}

		if ((addr >= 0x4004) && (addr <= 0x4007)) {
			switch (addr) {
				case PULSE2_DUTY_ENV:
					/* Set Duty */
					squareList[1].duty              = ((data & 0xC0) >> 0x6);
					/* Set haltFlag/envelope loop flag */
					squareList[1].env.loop_flag     = ((data & 0x20) >> 0x5);
					/* Set const flag */
					squareList[1].env.c_flag        = ((data & 0x10) >> 0x4);
					/* Set Volume */
					squareList[1].env.volume        = (data & 0xF);
					/* Load the envelope counter */
					squareList[1].env.down_cnt      = (data & 0xF) + 1;
				break;

				case PULSE2_SWEEP:
					/* Store all sweep information */
					/* write on this register result in reload the divider with period */
					squareList[1].swp.enable        = ((data & 0x80) >> 0x7);
					squareList[1].swp.period        = ((data & 0x70) >> 0x4);
					squareList[1].swp.neg_flag      = ((data & 0x08) >> 0x3);
					squareList[1].swp.shift_cnt     =  (data & 0x07);
					squareList[1].swp.divider_cnt   = squareList[1].swp.period;
				break;

				/* Set the low 8 bits on timer */
				case PULSE2_PERIOD_LOW:
					squareList[1].timer = ((squareList[1].timer & 0x700) | data);
					square_freq_output(SQUARE_WAVE_UNIT_2);
				break;

				/* load the high 3 bits of timer */
				case PULSE2_LEN_PE_HIGH:
					squareList[1].timer          = ((squareList[1].timer & 0xFF) | ((data & 0x7) << 8));
					/* It reset the envelope decay counter to 0xF(max) state */
					squareList[1].env.down_cnt   = 0xF;
					/* Load the lenght counter( find the proper value on table ) */
					squareList[1].len_cnt        = getLengthCnt(((data & 0xF8) >> 3)); 
					square_freq_output(SQUARE_WAVE_UNIT_2);
				break;


				default:	
					printf("Register 0x%X not implemented \n", addr);
				break;
			}
		}

		if ((addr >= 0x4008) && (addr <= 0x400B)) {
			switch (addr) {
				case TRIANGLE_CNT_LOAD:
					triangle.controlFlag = ((data & 0x80) >> 0x7);
					triangle.linear      =  (data & 0x7F);
				break;

				case TRIANGLE_UNUSED:

				break;

				case TRIANGLE_TMR_LOW:
					triangle.timer = ((triangle.timer & 0x700) | data);
					triangle.tmr_cnt = triangle.timer;
					triangle_freq_output();
				break;

				case TRIANGLE_TMR_HIGH:
					triangle.timer      = ((triangle.timer & 0xFF) | ((data & 0x7) << 8));
					triangle.len_cnt    = getLengthCnt(((data & 0xF8) >> 3)); 
					triangle_freq_output();
					triangle.tmr_cnt = triangle.timer;
					triangle.linear_cnt = triangle.linear;
					triangle.haltFlag   = 1;
				break;
			}
		}

		if ((addr >= 0x400C) && (addr <= 0x400F)) {
			switch (addr) {
				case NOISE_ENV:
					/* loop env/disable length */
					noise.env.loop_flag             = ((data & 0x20) >> 0x5);
					/* envelop decay flag */
					noise.env.c_flag       	        = ((data & 0x10) >> 0x4);
					/* Set Volume */
					noise.env.volume                = (data & 0xF);
				break;

				case NOISE_UNUSED:

				break;

				case NOISE_L_PERIOD:
					/* Loop Noise */
					noise.mode                      = ((data & 0x80) >> 0x7);
					/* Noise Period */
					noise.timer                     = noise_lookup_tbl[(data & 0xF)];
					noise_out_freq();
				break;

				case NOISE_LEN_CNT:
					/* Load length counter from the lookup table */
					noise.len_cnt                   = getLengthCnt((data & 0xF8) >> 0x3);
				break;
			}
		}

		if ((addr >= 0x4010) && (addr <= 0x4013)) {
			switch (addr) {
				case DMC_ILF:
					dmc.irq = (data & 0x80);
					dmc.loop = (data & 0x40);
					dmc.freq = dmc_rate[(data & 0x0F)];
					dmc_update_freq();
				break;

				case DMC_LOAD_CNT:
					dmc.directLoad = (data & 0x7F);
					dmc.shift = 8;
				break;

				case DMC_SAMPLE_ADDR:
					dmc.addr = 0xC000 + (data * 64);
					dmc.addrCnt = dmc.addr;
				break;

				case DMC_SAMPLE_LEN:
					dmc.size = (data * 16) + 1;
					dmc.sizeCnt = dmc.size;
				break;
			}
		}

		/* Enable or Disable Channel individual */
		if (addr == STATUS_REGISTER) {
			/* If the channel is set with 0, silence the channel and lenght count goes to 0 */
			apu.dmc_flag         = ((data & 0x10) >> 0x4);
			apu.noise_flag       = ((data & 0x08) >> 0x3);
			apu.triangle_flag    = ((data & 0x04) >> 0x2);
			apu.pulse_channel_2  = ((data & 0x02) >> 0x1);
			apu.pulse_channel_1  =  (data & 0x01);

		}

		/* Frame Counter, it drives the envelope, sweep and lenght count */
		if (addr == FRAME_CNT_REG) {
			apu.frame_cnt_mode   = ((data & 0x80) >> 0x7);
			apu.irq_flag         = ((data & 0x40) >> 0x6);
			apu.dmc_interrupt    = 0x0;
		}

	}

	memory[addr] = data;
}

/* verify if it's over */
unsigned char macgyver_var;

/* Instruction set */
void adc_imm() {
	EA = PC++;
    
	unsigned char data = memoryRead(EA);
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void adc_zp() {
	EA = memoryRead(PC++);
    
	unsigned char data = memoryRead(EA);
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}

void adc_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    
	unsigned char data = memoryRead(EA);
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void adc_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void adc_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void adc_absy() {
	
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void adc_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
    
	unsigned char data = memoryRead(EA);
	temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}


void adc_indy() {
 	
	int temp = memoryRead(PC++);
	int data2 = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data2 = memoryRead(temp) + (memoryRead(data2) * 0x100);
	EA = data2 + Y;
    
	unsigned char data = memoryRead(EA);
	temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void and_imm() {
	EA = PC++;
    
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void and_zp() {
	EA = memoryRead(PC++);
    
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}

void and_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void and_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void and_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void and_absy() {
    
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void and_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
    
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void and_indy() {
    
	int temp = memoryRead(PC++);
	int data2 = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data2 = memoryRead(temp) + (memoryRead(data2) * 0x100);
	EA = data2 + Y;
    
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}


void asl_a() {
    if (A & 0x80) {P |= 0x1;} else {P &= ~0x1;}
	A <<= 1;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void asl_zp() {
	EA = memoryRead(PC++);
	unsigned char data = memoryRead(EA);
	if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
	data <<= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void asl_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	unsigned char data = memoryRead(EA);
	if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
	data <<= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void asl_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
	data <<= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void asl_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
	data <<= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 7;
}

void bcc() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if (!(P & 0x1))
	{
	    if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	tick_count += 2;
}

void bcs() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if ((P & 0x1) == 0x1)
	{
	    if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	tick_count += 2;
}

void beq() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if ((P & 0x2) == 0x2)
	{
	    if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	tick_count += 2;
}

void bit_zp() {
	EA = memoryRead(PC++);
	unsigned char data = memoryRead(EA);
	if (!(data & A)) {P |= 0x2;} else {P &= ~0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	if (data & 0x40) {P |= 0x40;} else {P &= ~0x40;}
	tick_count += 3;
}

void bit_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (!(data & A)) {P |= 0x2;} else {P &= ~0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	if (data & 0x40) {P |= 0x40;} else {P &= ~0x40;}
	tick_count += 4;
}

void bmi() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if ((P & 0x80) == 0x80)
	{
	    if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	tick_count += 2;
}

void bne() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if (!(P & 0x2))
	{
        if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	
	tick_count += 2;
}

void bpl() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if (!(P & 0x80))
	{
	    if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	tick_count += 2;
}

void brk() {

	PC ++;
    writeMemory(S + 0x100, PC >> 8);
	S = (S - 1) & 0xFF;
    writeMemory(S + 0x100, PC & 0xFF);
	S = (S - 1) & 0xFF;
	P |= 0x10;
    writeMemory(S + 0x100, P);
	S = (S - 1) & 0xFF;
	P |= 0x4;
	PC = memory[0xFFFE] + (memory[0xFFFF] * 0x100);
	tick_count += 7;
}

void bvc() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if (!(P & 0x40))
	{
	    if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	tick_count += 2;
}

void bvs() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if ((P & 0x40) == 0x40)
	{
	    if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	tick_count += 2;
}

void clc() { P &=~0x1;  tick_count += 2; }
void cld() { P &=~0x8;  tick_count += 2; }
void cli() { P &=~0x4;  tick_count += 2; }
void clv() { P &=~0x40; tick_count += 2; }


void cmp_imm() {
	EA = PC++;
    
	unsigned char data = memoryRead(EA);
	int temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}


void cmp_zp() {
	EA = memoryRead(PC++);
    
	unsigned char data = memoryRead(EA);
	int temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}


void cmp_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    
	unsigned char data = memoryRead(EA);
	int temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void cmp_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	int temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void cmp_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	int temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void cmp_absy() {
    
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	int temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void cmp_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
    
	unsigned char data = memoryRead(EA);
	temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}


void cmp_indy() {
    
	int temp = memoryRead(PC++);
	int data2 = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data2 = memoryRead(temp) + (memoryRead(data2) * 0x100);
	EA = data2 + Y;
    
	unsigned char data = memoryRead(EA);
	temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void cpx_imm() {
	EA = PC++;
	unsigned char data = memoryRead(EA);
	int temp = X - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void cpx_zp() {
	EA = memoryRead(PC++);
	unsigned char data = memoryRead(EA);
	int temp = X - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}


void cpx_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	unsigned char data = memoryRead(EA);
	int temp = X - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void cpy_imm() {
	EA = PC++;
	unsigned char data = memoryRead(EA);
	int temp = Y - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}


void cpy_zp() {
	EA = memoryRead(PC++);
	unsigned char data = memoryRead(EA);
	int temp = Y - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}


void cpy_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	unsigned char data = memoryRead(EA);
	int temp = Y - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void dec_zp() {
	EA = memoryRead(PC++);
	writeMemory(EA, (memoryRead(EA) - 1) & 0xFF);
	unsigned char data = memoryRead(EA);
    if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void dec_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	writeMemory(EA, (memoryRead(EA) - 1) & 0xFF);
	unsigned char data = memoryRead(EA);
    if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void dec_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	writeMemory(EA, (memoryRead(EA) - 1) & 0xFF);
	unsigned char data = memoryRead(EA);
    if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void dec_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
	writeMemory(EA, (memoryRead(EA) - 1) & 0xFF);
	unsigned char data = memoryRead(EA);
    if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 7;
}

void dex() {
	X = (X - 1) & 0xFF;
	if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void dey() {
	Y = (Y - 1) & 0xFF;
	if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void eor_imm() {
	EA = PC++;
    
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void eor_zp() {
	EA = memoryRead(PC++);
    
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}

void eor_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void eor_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void eor_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void eor_absy() {
    
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void eor_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
    
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void eor_indy() {
    
	int temp = memoryRead(PC++);
	int data2 = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data2 = memoryRead(temp) + (memoryRead(data2) * 0x100);
	EA = data2 + Y;
    
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void inc_zp() {
	EA = memoryRead(PC++);
	writeMemory(EA, (memoryRead(EA) + 1) & 0xFF);
	unsigned char data = memoryRead(EA);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void inc_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	writeMemory(EA, (memoryRead(EA) + 1) & 0xFF);
	unsigned char data = memoryRead(EA);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void inc_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;

	writeMemory(EA, (memoryRead(EA) + 1) & 0xFF);
	unsigned char data = memoryRead(EA);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void inc_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;

	writeMemory(EA, (memoryRead(EA) + 1) & 0xFF);
	unsigned char data = memoryRead(EA);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 7;
}

void inx() {
	X = (X + 1) & 0xFF;
	if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void iny() {
	Y = (Y + 1) & 0xFF;
	if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void jmp_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	PC = EA; tick_count += 3;
}

void jmp_ind() {
	int temp = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	int data = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	EA = memoryRead(temp) + (memoryRead(data) * 0x100);
	PC = EA; tick_count += 5;
}

void jsr() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;

    writeMemory(S + 0x100, (PC - 1) >> 8);
	S = (S - 1) & 0xFF;
    writeMemory(S + 0x100, (PC - 1) & 0xFF);
	S = (S - 1) & 0xFF;
	PC = EA;
	tick_count += 6;
}

void lda_imm() {
	EA = PC++;
    
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}


void lda_zp() {
	EA = memoryRead(PC++);
    
    
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}


void lda_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void lda_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void lda_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void lda_absy() {
    
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void lda_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
    
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}


void lda_indy() {
    
	int temp = memoryRead(PC++);
	int data2 = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data2 = memoryRead(temp) + (memoryRead(data2) * 0x100);
	EA = data2 + Y;
    
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void ldx_imm() {
	EA = PC++;
    
	X = memoryRead(EA);
	if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void ldx_zp() {
	EA = memoryRead(PC++);
    
	X = memoryRead(EA);
	if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}

void ldx_zpy() {
	EA = (memoryRead(PC++) + Y) & 0xFF;
    
	X = memoryRead(EA);
	if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void ldx_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    
	X = memoryRead(EA);
	if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void ldx_absy() {
    
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    
	X = memoryRead(EA);
	if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void ldy_imm() {
	EA = PC++;
    
	Y = memoryRead(EA);
	if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void ldy_zp() {
	EA = memoryRead(PC++);
    
	Y = memoryRead(EA);
	if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}

void ldy_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    
	Y = memoryRead(EA);
	if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void ldy_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    
	Y = memoryRead(EA);
	if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void ldy_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    
	Y = memoryRead(EA);
	if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void lsr_a() {
	if (A & 0x1) {P |= 0x1;} else {P &= ~0x1;}
	A >>= 1;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void lsr_zp() {
	EA = memoryRead(PC++);
	unsigned char data = memoryRead(EA);
	if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
	data >>= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void lsr_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	unsigned char data = memoryRead(EA);
	if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
	data >>= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void lsr_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
	data >>= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void lsr_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
	data >>= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 7;
}

void nop() { tick_count += 2; }


void ora_imm() {
	EA = PC++;
    
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}


void ora_zp() {
	EA = memoryRead(PC++);
    
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}


void ora_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void ora_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void ora_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void ora_absy() {
    
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void ora_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
    
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}


void ora_indy() {
    
	int temp = memoryRead(PC++);
	int data2 = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data2 = memoryRead(temp) + (memoryRead(data2) * 0x100);
	EA = data2 + Y;
    
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void pha() {
    writeMemory(S + 0x100, A);
	S = (S - 1) & 0xFF;
	tick_count += 3;
}

void php() {
    writeMemory(S + 0x100, P | 0x10);
	S = (S - 1) & 0xFF;
	tick_count += 3;
}

void pla() {
    S = (S + 1) & 0xFF;
	A = memoryRead(S + 0x100);
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void plp() {
    S = (S + 1) & 0xFF;
	P = memoryRead(S + 0x100) | 0x20;
	tick_count += 4;
}

void rol_a() {
	if (P & 0x1)
	{
		if (A & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		A = (A << 1) | 0x1;
	}
	else
	{
		if (A & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		A <<= 1;
	}
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void rol_zp() {
	EA = memoryRead(PC++);
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data = (data << 1) | 0x1;
	}
	else
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data <<= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void rol_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data = (data << 1) | 0x1;
	}
	else
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data <<= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void rol_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data = (data << 1) | 0x1;
	}
	else
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data <<= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void rol_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data = (data << 1) | 0x1;
	}
	else
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data <<= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 7;	
}

void ror_a() {
	if (P & 0x1)
	{
	    if (A & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		A = (A >> 1) | 0x80;
	}
	else
	{
	    if (A & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		A >>= 1;
	}
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void ror_zp() {
	EA = memoryRead(PC++);
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
		if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data = (data >> 1) | 0x80;
	}
	else
	{
	    if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data >>= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}


void ror_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
		if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data = (data >> 1) | 0x80;
	}
	else
	{
	    if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data >>= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}


void ror_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
		if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data = (data >> 1) | 0x80;
	}
	else
	{
	    if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data >>= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}


void ror_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
		if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data = (data >> 1) | 0x80;
	}
	else
	{
	    if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data >>= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 7;
}

void rti() {
    S = (S + 1) & 0xFF;
	P = memoryRead(S + 0x100) | 0x20;
	S = (S + 1) & 0xFF;
	PC = memoryRead(S + 0x100);
	S = (S + 1) & 0xFF;
	PC |= (memoryRead(S + 0x100) * 0x100);
	tick_count += 6;
}

void rts() {
    S = (S + 1) & 0xFF;
	PC = memoryRead(S + 0x100);
	S = (S + 1) & 0xFF;
	PC |= (memoryRead(S + 0x100) * 0x100);
	PC++;
	tick_count += 6;
}

void sbc_imm() {
	EA = PC++;
    
	unsigned char data = memoryRead(EA) ^ 0xFF;
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void sbc_zp() {
	EA = memoryRead(PC++);
    
	unsigned char data = memoryRead(EA) ^ 0xFF;
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}

void sbc_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    
	unsigned char data = memoryRead(EA) ^ 0xFF;
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void sbc_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    
	unsigned char data = memoryRead(EA) ^ 0xFF;
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void sbc_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    
	unsigned char data = memoryRead(EA) ^ 0xFF;
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void sbc_absy() {
    
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    
	unsigned char data = memoryRead(EA) ^ 0xFF;
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void sbc_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
    
	unsigned char data = memoryRead(EA) ^ 0xFF;
	temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void sbc_indy() {
    
	int temp = memoryRead(PC++);
	int data2 = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data2 = memoryRead(temp) + (memoryRead(data2) * 0x100);
	EA = data2 + Y;
    
	unsigned char data = memoryRead(EA) ^ 0xFF;
	temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void sec() {
	P |= 0x1;
	tick_count += 2; 
}

void sed() {
	P |= 0x8;
	tick_count += 2; 
}

void sei() {
	P |= 0x4;
	tick_count += 2; 
}

void sta_zp() {
	EA = memoryRead(PC++);
	writeMemory(EA, A);
	tick_count += 3;
}

void sta_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	writeMemory(EA, A);
	tick_count += 4;
}


void sta_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	writeMemory(EA, A);
	tick_count += 4;
}


void sta_absx() {
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
	writeMemory(EA, A);
	tick_count += 5;
}


void sta_absy() {
    
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
	writeMemory(EA, A);
	tick_count += 5;
}

void sta_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
	writeMemory(EA, A);
	tick_count += 6;
}

void sta_indy() {
    
	int temp = memoryRead(PC++);
	int data = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data = memoryRead(temp) + (memoryRead(data) * 0x100);
	EA = data + Y;
	writeMemory(EA, A);
	tick_count += 6;
}


void stx_zp() {
	EA = memoryRead(PC++);
	writeMemory(EA, X);
	tick_count += 3;
}

void stx_zpy() {
	EA = (memoryRead(PC++) + Y) & 0xFF;
	writeMemory(EA, X);
	tick_count += 4;
}

void stx_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	writeMemory(EA, X);
	tick_count += 4;
}

void sty_zp() {
	EA = memoryRead(PC++);
	writeMemory(EA, Y);
	tick_count += 3;
}

void sty_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	writeMemory(EA, Y);
	tick_count += 4;
}

void sty_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	writeMemory(EA, Y);
	tick_count += 4;
}

void tax() {
	X = A;
    if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void tay() {
	Y = A;
    if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void tsx() {
	X = S;
    if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void txa() {
	A = X;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void txs() {
	S = X;
	tick_count += 2;
}

void tya() {
	A = Y;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void imm() { //immediate
	EA = PC++;
}

void zp() { //zero-page
	EA = memoryRead(PC++);
}

void zpx() { //zero-page,X
	EA = (memoryRead(PC++) + X) & 0xFF;
}

void zpy() { //zero-page,Y
	EA = (memoryRead(PC++) + Y) & 0xFF;
}

void rel() { //relative for branch ops (8-bit immediate value, sign-extended)
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}
}

void abso() { //absolute
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
}

void absx() { //absolute,X
    
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
}

void absy() { //absolute,Y
    
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
}

void ind() { //indirect
	int temp = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	int data = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	EA = memoryRead(temp) + (memoryRead(data) * 0x100);
}

void indx() { //(indirect,X)
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
}

void indy() { //(indirect),Y
    
	int temp = memoryRead(PC++);
	int data = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data = memoryRead(temp) + (memoryRead(data) * 0x100);
	EA = data + Y;
}


void IRQ() {
	//Maskable Interrupt
    writeMemory(S + 0x100, PC >> 8);
	S = (S - 1) & 0xFF;
	writeMemory(S + 0x100, PC & 0xFF);
	S = (S - 1) & 0xFF;
	P |= 0x10;
	writeMemory(S + 0x100, P);
	S = (S - 1) & 0xFF;
	P |= 0x4;
	PC = memory[0xFFFE] + (memory[0xFFFF] * 0x100);
	tick_count += 7;
}


void NMI() {
	//Non-Maskable Interrupt
    writeMemory(S + 0x100, PC >> 8);
	S = (S - 1) & 0xFF;
	writeMemory(S + 0x100, PC & 0xFF);
	S = (S - 1) & 0xFF;
	P |= 0x10;
    writeMemory(S + 0x100, P);
	S = (S - 1) & 0xFF;
	P |= 0x4;
	PC = memory[0xFFFA] + (memory[0xFFFB] * 0x100);
	tick_count += 7;
}

void not_imp() {
	printf("opcode not implemented!\n");
}

void macgyver() { macgyver_var = 1; }

void (*op[0x100])() = {
																/* Set Function Pointer Vector */
	/*       0          1      2       3      4         5        6       7     8      9        A     B       C           D         E       F    */
			brk,    ora_indx, not_imp,   not_imp,  not_imp,    ora_zp,  asl_zp,  not_imp, php, ora_imm,  asl_a, not_imp,   not_imp,     ora_abso, asl_abso, not_imp, // 0
			bpl,    ora_indy, not_imp,   not_imp,  not_imp,    ora_zpx, asl_zpx, not_imp, clc, ora_absy, not_imp,  not_imp,   not_imp,     ora_absx, asl_absx, not_imp, // 1
			jsr,    and_indx, not_imp,   not_imp,  bit_zp,  and_zp,  rol_zp,  not_imp, plp, and_imm,  rol_a, not_imp,   bit_abso, and_abso, rol_abso, not_imp, // 2  
			bmi,    and_indy, not_imp,   not_imp,  not_imp,    and_zpx, rol_zpx, not_imp, sec, and_absy, not_imp,  not_imp,   not_imp,     and_absx, rol_absx, not_imp, // 3
		    rti,    eor_indx, not_imp,   not_imp,  not_imp,    eor_zp,  lsr_zp,  not_imp, pha, eor_imm,  lsr_a, not_imp,   jmp_abso, eor_abso, lsr_abso, not_imp, // 4
			bvc,    eor_indy, not_imp,   not_imp,  not_imp,    eor_zpx, lsr_zpx, not_imp, cli, eor_absy, not_imp,  not_imp,   not_imp,     eor_absx, lsr_absx, not_imp, // 5
			rts,    adc_indx, not_imp,   not_imp,  not_imp,    adc_zp,  ror_zp,  not_imp, pla, adc_imm,  ror_a, not_imp,   jmp_ind,  adc_abso, ror_abso, not_imp, // 6
			bvs,    adc_indy, not_imp,   not_imp,  not_imp,    adc_zpx, ror_zpx, not_imp, sei, adc_absy, not_imp,  not_imp,   not_imp,     adc_absx, ror_absx, not_imp, // 7
			not_imp,   sta_indx, not_imp,   not_imp,  sty_zp,  sta_zp,  stx_zp,  not_imp, dey, not_imp,     txa,   not_imp,   sty_abso, sta_abso, stx_abso, not_imp, // 8
			bcc,    sta_indy, not_imp,   not_imp,  sty_zpx, sta_zpx, stx_zpy, not_imp, tya, sta_absy, txs,   not_imp,   not_imp,     sta_absx, not_imp,     not_imp, // 9
			ldy_imm,lda_indx, ldx_imm,not_imp,  ldy_zp,  lda_zp,  ldx_zp,  not_imp, tay, lda_imm,  tax,   not_imp,   ldy_abso, lda_abso, ldx_abso, not_imp, // A
			bcs,    lda_indy, not_imp,   not_imp,  ldy_zpx, lda_zpx, ldx_zpy, not_imp, clv, lda_absy, tsx,   not_imp,   ldy_absx, lda_absx, ldx_absy, not_imp, // B
			cpy_imm,cmp_indx, not_imp,   not_imp,  cpy_zp,  cmp_zp,  dec_zp,  not_imp, iny, cmp_imm,  dex,   not_imp,   cpy_abso, cmp_abso, dec_abso, not_imp, // C
			bne,    cmp_indy, not_imp,   not_imp,  not_imp,    cmp_zpx, dec_zpx, not_imp, cld, cmp_absy, not_imp,  not_imp,   not_imp,     cmp_absx, dec_absx, not_imp, // D
			cpx_imm,sbc_indx, not_imp,   not_imp,  cpx_zp,  sbc_zp,  inc_zp,  not_imp, inx, sbc_imm,  nop,   sbc_imm,cpx_abso, sbc_abso, inc_abso, not_imp, // E
			beq,    sbc_indy, not_imp,   not_imp,  not_imp,    sbc_zpx, inc_zpx, not_imp, sed, sbc_absy, nop,   not_imp,   not_imp,     sbc_absx, inc_absx, macgyver};// F 


void CPU_reset() {
    A = 0;
    X = 0;
    Y = 0;
    S = 0xFF;
    P = 0x20;
	PC = memory[0xFFFC] + (memory[0xFFFD] * 0x100);
	tick_count     = 0;
	macgyver_var   = 0;

	memset(&triangle, 0x0, sizeof(triangle_wave));
	noise.lfsr = 1;
}

void CPU_execute(int cycles) {
	unsigned char opcode;

	while (tick_count < cycles) {
		if (macgyver_var)
			break;
		
		

		opcode=memoryRead(PC++);

		(*op[opcode])();
	}

	tick_count = 0;
}
