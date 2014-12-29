#include "msp430G2553.h"

#define SCL_PIN BIT6
#define SDA_PIN BIT7
#define SCL_LO P1DIR |= SCL_PIN
#define SCL_HI P1DIR &= ~SCL_PIN
#define SDA_LO P1DIR |= SDA_PIN
#define SDA_HI P1DIR &= ~SDA_PIN

#define BUTTON BIT3

#define LCD_ADDRESS 0x7c
#define RGB_ADDRESS 0xc4
#define LCD_TOP_LINE 0x80
#define LCD_BOT_LINE 0xc0

void strncpy(char* dest, char* source, int bytes) {
	for (;bytes > 0; bytes--) {
		*dest = *source;
		dest++;
		source++;
	}
}

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

unsigned char rgb_default_data[] = {0x80,
		0x80, 0x0d, // 0, 1
		0xff, 0xff, 0xff, 0x00, // B, G, R, NC
		0xff, 0x00,  // 6, 7
		0xaa, // 8
};
void turn_on_led() {
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

char lcd_message[32] = "Clean - 01:00:00Unloader: Anting";
void lcd_write(unsigned char addr, char* string, int bytes) {
	i2c_start();
	i2c_send_byte(LCD_ADDRESS);

	i2c_send_byte(0x80);
	i2c_send_byte(addr);
	__delay_cycles(500);

	i2c_send_byte(0x40);
	i2c_send((unsigned char*)string, bytes);
	__delay_cycles(500);

	i2c_stop();
}

void lcd_init() {
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
	i2c_send_byte(LCD_TOP_LINE);
	__delay_cycles(500);

	i2c_send_byte(0x40);
	i2c_send((unsigned char*)lcd_message, 16);
	__delay_cycles(500);

	i2c_stop(); // restart
	__delay_cycles(500);
	lcd_write(LCD_BOT_LINE, lcd_message+16, 16);
}

unsigned char day_of_week;
void init_clock() {
	TACCTL0 = CCIE;
	TACCR0 = 32767;
	TACTL = TASSEL_2 | MC_2 | ID_3;
}

unsigned char dirty = 0;
void set_dirty() {
	strncpy(lcd_message, "Dirty", 5);
	strncpy(lcd_message+16, "                ", 16);
	lcd_write(LCD_TOP_LINE, lcd_message, 5);
	lcd_write(LCD_BOT_LINE, lcd_message+16, 16);
	set_led_color(0, 0xff, 0);
}

void set_clean() {
	strncpy(lcd_message, "Clean", 5);
	strncpy(lcd_message+16, "Unloader:", 9);
	char* unloader;
	switch (day_of_week >> 1) {
		case 0:
			unloader = "Ian   ";
			break;
		case 1:
			unloader = "Dan   ";
			break;
		case 2:
			unloader = "Jimmy ";
			break;
		case 3:
			unloader = "Ed    ";
			break;
		case 4:
			unloader = "Anting";
			break;
		case 5:
			unloader = "Simon ";
			break;
		case 6:
			unloader = "Ray   ";
			break;
	}
	strncpy(lcd_message+26, unloader, 6);
	lcd_write(LCD_TOP_LINE, lcd_message, 5);
	lcd_write(LCD_BOT_LINE, lcd_message+16, 16);
	set_led_color(0, 0, 0xff);
}

void main(void) {
	WDTCTL = WDTPW + WDTHOLD; 	// Stop watchdog timer

	BCSCTL1 = CALBC1_1MHZ; 		// Set CPU clock to 1MHz
	DCOCTL = CALDCO_1MHZ;

	P1OUT = 0;
	P1DIR |= BIT0; // LED

	P1REN |= BUTTON;
	P1OUT |= BUTTON;	// pull-up
	P1IE |= BUTTON;		// interrupt
	P1IES |= BUTTON;
	P1IFG &= ~BUTTON;

	__delay_cycles(50000);

	lcd_init();

	turn_on_led();
	set_led_color(0, 0, 0xff);

	init_clock();
	__enable_interrupt();
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {

	lcd_write(LCD_TOP_LINE+8, lcd_message+8, 8);

	char* ptr = lcd_message + 15;
	*ptr += 1;	// ptr = sec1
	if (*ptr != ':') return;
	*ptr = '0';
	ptr--;		// ptr = sec10
	*ptr += 1;
	if (*ptr != '6') return;
	*ptr = '0';
	ptr -= 2;	// ptr = min1
	*ptr += 1;
	if (*ptr != ':') return;
	*ptr = '0';
	ptr--;		// ptr = min10
	*ptr += 1;
	if (*ptr != '6') return;
	*ptr = '0';
	ptr -= 2;	// ptr = hour1
	*ptr += 1;
	if (*ptr == ':') {
		*ptr = '0';
		*(ptr-1) += 1;
		return;
	}
	if (*(ptr-1) == '1') {
		if (*ptr == '2') {
			day_of_week += 1;
			if (day_of_week == 14) {
				day_of_week = 0;
			}
			if (dirty == 0) set_clean();
			return;
		}
		if (*ptr == '3') {
			*(ptr-1) = '0';
			*ptr = '1';
			return;
		}
	}
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
	P1IFG &= ~BUTTON;
	P1IE &= ~BUTTON;
	dirty ^= 1;
	if (dirty == 0) {
		set_clean();
	}
	if (dirty == 1) {
		set_dirty();
	}
	P1IE |= BUTTON;
}

//#pragma vector=unused_interrupts
//__interrupt void unused(void) {
//
//}









