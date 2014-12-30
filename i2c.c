#include "i2c.h"

void measure() {
//	P1OUT &= ~(BIT0 + BIT6);
//	if (P1IN & BIT4) {
//		P1OUT |= BIT0;
//	}
//	if (P1IN & BIT5) {
//		P1OUT |= BIT6;
//	}
	__delay_cycles(5);
//	wait_for_button();
}

void i2c_start() {
	SCL_HI;
	SDA_HI;
	measure();
	SDA_LO;
	measure();
	SCL_LO;
	measure();
}

void i2c_stop() {
	SDA_LO;
	measure();
	SCL_HI;
	measure();
	SDA_HI;
	measure();
}

void i2c_send_byte(unsigned char byte) {
	int bits_left;
	for (bits_left = 8; bits_left > 0; bits_left--) {
		if ((byte >> (bits_left - 1)) & 1) {
			SDA_HI;
		} else {
			SDA_LO;
		}
		measure();
		SCL_HI;
		measure();
		SCL_LO;
		measure();
	}
	SDA_HI;
	measure();
	SCL_HI;
	measure();
	SCL_LO;
	measure();
}

void i2c_send(unsigned char* data, int length) {
	while (length > 0) {
		i2c_send_byte(*data);
		data++;
		length--;
	}
}
