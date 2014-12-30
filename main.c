#include "msp430G2553.h"
#include "i2c.h"
#include "lcd.h"

#define BUTTON	BIT3
#define TOUCH	BIT1
#define DIRTY_REG P2OUT	// store dirty state in unused register bit instead of RAM
#define DIRTY_BIT BIT0

char time[5] = "01:00";

int day_of_week = 0;	// last bit is AM/PM, other bits are day of week
int seconds = 0;
void init_clock() {
	TACCTL0 = CCIE;		// Enable TimerA interrupt
	TACCR0 = 32767; 	// Interrupt after this many ticks
	TACTL = TASSEL_2	// Use SMCLK (system clock)
			| MC_1		// Timer counts up to TACCR0
			| ID_3;		// Divide clock input by 8
}

void set_dirty() {
	lcd_write(LCD_TOP_LINE, "Dirty", 5);
	lcd_write(LCD_BOT_LINE, "                ", 16);
	set_led_color(0, 0xff, 0);
}

static const char unloaders[56] = ": Ian   : Dan   : Jimmy : Ed    : Anting: Simon : Ray   ";

void set_clean() {
	lcd_write(LCD_TOP_LINE, "Clean", 5);
	lcd_write(LCD_BOT_LINE, "Unloader", 9);
	lcd_write(LCD_BOT_LINE + 8,
			unloaders + ((day_of_week & (~1)) << 2),
			8);
	set_led_color(0, 0, 0xff);
}

void main(void) {
	WDTCTL = WDTPW + WDTHOLD; 	// Stop watchdog timer

	BCSCTL1 = CALBC1_1MHZ; 		// Set CPU clock to 1MHz
	DCOCTL = CALDCO_1MHZ;

	P1OUT = 0;
	P2DIR |= ~0;
	P2OUT = 0;
	P3DIR |= ~0;
	P3OUT = 0;
	P1DIR |= BIT0; // LED

	P1REN |= BUTTON | TOUCH;
	P1OUT |= BUTTON;	// pull-up
	P1IE |= BUTTON | TOUCH;		// interrupt
	P1IES |= BUTTON;
	P1IFG &= ~(BUTTON | TOUCH);

	__delay_cycles(50000);

	lcd_init();
	lcd_write(LCD_TOP_LINE + 6, "- HH:MM AM", 10);

	turn_on_led();
	set_led_color(0, 0, 0xff);

	set_clean();

	init_clock();

	_BIS_SR(CPUOFF	// Turn off CPU to save power
			+ GIE);	// Enable interrupts
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {

	P1OUT ^= BIT0;
	seconds += 1;
	if (seconds != 4) return;

	seconds = 0;
	lcd_write(LCD_TOP_LINE + 8, time, 5);

	char* ptr = time + 4;
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
			if (!(DIRTY_REG & DIRTY_BIT)) set_clean();
			if (day_of_week & 1) {
				lcd_write(LCD_TOP_LINE + 14, "P", 1);
			} else {
				lcd_write(LCD_TOP_LINE + 14, "A", 1);
			}
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
	P1IFG &= ~(BUTTON + TOUCH);
	P1IE &= ~(BUTTON + TOUCH);
	DIRTY_REG ^= DIRTY_BIT;
	if (DIRTY_REG & DIRTY_BIT) {
		set_dirty();
	} else {
		set_clean();
	}
	P1IE |= (BUTTON + TOUCH);
}








