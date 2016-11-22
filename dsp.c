#include "dsp.h"

/** sound dsp */
dsp s_dsp;
 
/** out samples */
short samples[8][MAX_SAMPLES];

void dsp_write(unsigned char reg, unsigned char data) {
	reg = reg & 0x7F;
	unsigned char channel = reg & 0x0F;

	printf("REG: 0x%X - DATA: 0x%X\n", reg, data);

	/* is a channel register */
	if ((channel >= 0) && (channel <= 0x9)) {
		unsigned char cNum = (reg >> 4);
		switch (channel) {
			case 0x0:
				s_dsp.channelInfo[cNum].VnVOLL = data;
			break;

			case 0x1:
				s_dsp.channelInfo[cNum].VnVOLR = data;
			break;

			case 0x2:
				s_dsp.channelInfo[cNum].VnPitchL = data;
			break;

			case 0x3:
				s_dsp.channelInfo[cNum].VnPitchH = data;
			break;

			case 0x4:
				s_dsp.channelInfo[cNum].VnSRCN = data;
			break;

			case 0x5:
				s_dsp.channelInfo[cNum].VnADSR1 = data;
			break;

			case 0x6:
				s_dsp.channelInfo[cNum].VnADSR2 = data;
			break;

			case 0x7:
				s_dsp.channelInfo[cNum].VnGAIN = data;
			break;

			case 0x8:
				s_dsp.channelInfo[cNum].VnENVX  = data;
			break;

			case 0x9:
				s_dsp.channelInfo[cNum].VnOUTX = data;
			break;
		}
	}

	/* Echo FIR filter coefficent */
	if (channel == 0xF) {
		unsigned char cFir = (reg >> 4);
		s_dsp.FIR[cFir] = data;
	}

	switch (reg) {
		case 0x0C:
			s_dsp.MVOLL = data;
		break;

		case 0x1C:
			s_dsp.MVORR = data;
		break;

		case 0x2C:
			s_dsp.EVOLL = data;
		break;

		case 0x3C:
			s_dsp.EVOLR = data;
		break;

		case 0x4C: {
			s_dsp.KON = data;
			for (unsigned char i = 0; i < 8; i++) {
				/** check the channel that should be on */
				if ((1 << i) & data) {
					s_dsp.channel[i].sampleTablePtr = (s_dsp.DIR << 8) + (s_dsp.channelInfo[i].VnSRCN << 2);
					s_dsp.channel[i].sampleTablePtr =  spc_mem[s_dsp.channel[i].sampleTablePtr] | (spc_mem[s_dsp.channel[i].sampleTablePtr+1] << 8);
					s_dsp.channel[i].enable = 1;
					s_dsp.ENDX &= ~(1 << i);
				}
			}
		}
		break;

		case 0x5C:
			s_dsp.KOFF = data;
		break;

		case 0x6C:
			s_dsp.FLG = data;
		break;

		case 0x7C:
			s_dsp.ENDX = data;
		break;

		case 0x0D:
			s_dsp.EFB = data;
		break;

		case 0x2D:
			s_dsp.PMON = data;
		break;

		case 0x3D:
			s_dsp.NON = data;
		break;

		case 0x4D:
			s_dsp.EON = data;
		break;

		case 0x5D:
			s_dsp.DIR = data;
		break;

		case 0x6D:
			s_dsp.ESA = data;
		break;

		case 0x7D:
			s_dsp.EDL = data;
		break;
	}

}

unsigned char dsp_read(unsigned char reg) {
	reg = reg & 0x7F;

	unsigned char channel = reg & 0x0F;

	/* is a channel register */
	if ((channel >= 0) && (channel <= 0x9)) {
		unsigned char cNum = (reg >> 4);
		switch (channel) {
			case 0x0:
				return s_dsp.channelInfo[cNum].VnVOLL;
			break;

			case 0x1:
				return s_dsp.channelInfo[cNum].VnVOLR;
			break;

			case 0x2:
				return s_dsp.channelInfo[cNum].VnPitchL;
			break;

			case 0x3:
				return s_dsp.channelInfo[cNum].VnPitchH;
			break;

			case 0x4:
				return s_dsp.channelInfo[cNum].VnSRCN;
			break;

			case 0x5:
				return s_dsp.channelInfo[cNum].VnADSR1;
			break;

			case 0x6:
				return s_dsp.channelInfo[cNum].VnADSR2;
			break;

			case 0x7:
				return s_dsp.channelInfo[cNum].VnGAIN;
			break;

			case 0x8:
				return s_dsp.channelInfo[cNum].VnENVX;
			break;

			case 0x9:
				return s_dsp.channelInfo[cNum].VnOUTX ;
			break;
		}
	}


	/* Echo FIR filter coefficent */
	if (channel == 0xF) {
		unsigned char cFir = (reg >> 4);
		return s_dsp.FIR[cFir];
	}

	switch (reg) {
		case 0x0C:
			return s_dsp.MVOLL;
		break;

		case 0x1C:
			return s_dsp.MVORR;
		break;

		case 0x2C:
			return s_dsp.EVOLL;
		break;

		case 0x3C:
			return s_dsp.EVOLR;
		break;

		case 0x4C:
			return s_dsp.KON;
		break;

		case 0x5C:
			return s_dsp.KOFF;
		break;

		case 0x6C:
			return s_dsp.FLG;
		break;

		case 0x7C:
			return s_dsp.ENDX;
		break;

		case 0x0D:
			return s_dsp.EFB;
		break;

		case 0x2D:
			return s_dsp.PMON;
		break;

		case 0x3D:
			return s_dsp.NON;
		break;

		case 0x4D:
			return s_dsp.EON;
		break;

		case 0x5D:
			return s_dsp.DIR;
		break;

		case 0x6D:
			return s_dsp.ESA;
		break;

		case 0x7D:
			return s_dsp.EDL;
		break;
	}

	return 0;
}

void doSamples() {
	short out[MAX_SAMPLES];
	unsigned int pos = 0;
	unsigned char end_bit = 0;
	/** Process each channel sample */
	for (int i = 0; i < 8; i++) {
		memset(out, 0, MAX_SAMPLES);
		pos = 0;

		if (s_dsp.channel[i].enable) {
			while (end_bit == 0) {
				unsigned char end = spc_mem[s_dsp.channel[i].sampleTablePtr] & 0x1;
				unsigned char loop = spc_mem[s_dsp.channel[i].sampleTablePtr] & 0x2;
				unsigned char filter = (spc_mem[s_dsp.channel[i].sampleTablePtr] >> 2) & 0x3;
				unsigned char range = (spc_mem[s_dsp.channel[i].sampleTablePtr] >> 4);

				/* next byte */
				s_dsp.channel[i].sampleTablePtr++;

				for (int counter = 0; counter < 8; counter++) {
					unsigned char temp = spc_mem[s_dsp.channel[i].sampleTablePtr++];
					short nibble = (temp >> 4) & 0xF;

					/** negative nibble, so make it 16 bit value negative */
					if (nibble >= 8) {
						nibble = nibble | 0xFFF0;
					}

					/** store and increment to the next position in the buffer */
					out[pos++] = nibble << range;

					/** get the next nibble */
					nibble = temp & 0xF;

					/** negative nibble, so make it 16 bit value negative */
					if (nibble >= 8) {
						nibble = nibble | 0xFFF0;
					}
					/** store and increment to the next position in the buffer */
					out[pos++] = nibble << range;
				}
			}
			/** test */
			memcpy(samples[i], out, MAX_SAMPLES);
		}
	}
}
