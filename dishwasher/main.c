#include "msp430G2553.h"
#include "i2c.h"
#include "lcd.h"

#define BUTTON BIT3

char lcd_message[32] = "Clean - 01:00:00                ";
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
	P2DIR |= ~0;
	P1DIR |= BIT0; // LED

	P1REN |= BUTTON;
	P1OUT |= BUTTON;	// pull-up
	P1IE |= BUTTON;		// interrupt
	P1IES |= BUTTON;
	P1IFG &= ~BUTTON;

	__delay_cycles(50000);

	lcd_init(lcd_message);

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









