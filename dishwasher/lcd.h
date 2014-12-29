#ifndef LCD_H_
#define LCD_H_

#define LCD_ADDRESS 0x7c
#define RGB_ADDRESS 0xc4
#define LCD_TOP_LINE 0x80
#define LCD_BOT_LINE 0xc0

void turn_on_led();
void set_led_color(unsigned char red, unsigned char green, unsigned char blue);
void cycle_colors_forever();
void lcd_write(unsigned char addr, char* string, int bytes);
void lcd_init();

#endif /* LCD_H_ */
