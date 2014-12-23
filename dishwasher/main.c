#include "msp430G2553.h"

#define SCL_PIN BIT6
#define SDA_PIN BIT7
#define SCL_LO P1DIR |= SCL_PIN
#define SCL_HI P1DIR &= ~SCL_PIN
#define SDA_LO P1DIR |= SDA_PIN
#define SDA_HI P1DIR &= ~SDA_PIN

#define LCD_ADDRESS 0x7c
#define RGB_ADDRESS 0xc4

void wait_for_button() {
	__delay_cycles(200000);
	while(P1IN & BIT3);
}

void measure() {
//	P1OUT &= ~(BIT0 + BIT6);
//	if (P1IN & BIT4) {
//		P1OUT |= BIT0;
//	}
//	if (P1IN & BIT5) {
//		P1OUT |= BIT6;
//	}
//	__delay_cycles(5000);
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

void turn_on_led() {
	unsigned char rgb_default_data[] = {0x80,
			0x80, 0x0d, // 0, 1
			0xff, 0xff, 0xff, 0x00, // B, G, R, NC
			0xff, 0x00,  // 6, 7
			0xaa, // 8
	};
	i2c_start();
	i2c_send_byte(RGB_ADDRESS);
	i2c_send(rgb_default_data, 10);
	i2c_stop();
}
void set_led_color(unsigned char red, unsigned char green, unsigned char blue) {
	i2c_start();
	i2c_send_byte(RGB_ADDRESS);
	i2c_send_byte(0xa2);
	i2c_send_byte(blue);
	i2c_send_byte(green);
	i2c_send_byte(red);
	i2c_stop();
}
void cycle_colors_forever() {
	unsigned char color[] = {0xff, 0x00, 0x00};
	int prev = 0;
	int next = 1;
	int ctr;
	for (;;) {
		for (ctr = 255; ctr > 0; ctr--) {
			color[next]++;
			set_led_color(color[0], color[1], color[2]);
			__delay_cycles(100);
		}
		for (ctr = 255; ctr > 0; ctr--) {
			color[prev]--;
			set_led_color(color[0], color[1], color[2]);
			__delay_cycles(100);
		}
		prev++;
		next++;
		if (prev == 3) prev = 0;
		if (next == 3) next = 0;
	}
}

void lcd_write(char* string) { // string must be 32 chars
	i2c_start();
	i2c_send_byte(LCD_ADDRESS);

	i2c_send_byte(0x80);
	i2c_send_byte(0x3f); // function set, 2 line
	__delay_cycles(5000);
	i2c_send_byte(0x80);
	i2c_send_byte(0x3f); // function set, 2 line
	__delay_cycles(5000);
	i2c_send_byte(0x80);
	i2c_send_byte(0x3f); // function set, 2 line
	__delay_cycles(50000);

	i2c_send_byte(0x80);
	i2c_send_byte(0x0c); // display on cursor off
	__delay_cycles(500);
	i2c_send_byte(0x80);
	i2c_send_byte(0x01); // clear disp
	__delay_cycles(50000);
	i2c_send_byte(0x80);
	i2c_send_byte(0x60); // entry mode set
	__delay_cycles(500);

	i2c_send_byte(0x80);
	i2c_send_byte(0x80); // top line
	__delay_cycles(500);

	i2c_send_byte(0x40);
	i2c_send((unsigned char*)string, 16);
	__delay_cycles(500);

	string += 16;
	i2c_stop(); // restart
	__delay_cycles(500);
	i2c_start();
	i2c_send_byte(LCD_ADDRESS);

	i2c_send_byte(0x80);
	i2c_send_byte(0xc0); // bottom line
	__delay_cycles(500);

	i2c_send_byte(0x40);
	i2c_send((unsigned char*)string, 16);
	__delay_cycles(500);

	i2c_stop();
}


void main(void) {
	WDTCTL = WDTPW + WDTHOLD;
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	P1OUT = 0;

	SCL_HI;
	SDA_HI;
	measure();

	__delay_cycles(50000);
	lcd_write("Hello World!    This5678lollabcd");

	turn_on_led();
	cycle_colors_forever();

	// blink_forever(500, 4000);
}

