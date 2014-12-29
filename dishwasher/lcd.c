#include "lcd.h"

const unsigned char rgb_default_data[] = {0x80,
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

void lcd_init(char* lcd_message) {
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
