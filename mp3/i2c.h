#ifndef I2C_H
#define I2C_H

#include "lpc_types.h"

#define LCD_DISPLAY_ADDRESS 0x3B
#define KEYPAD_ADDRESS 0x21
#define LCD_CHAR(x) x + 0x80

void init_i2c(void);

Status write_i2c(uint8_t addr, void* data_out, unsigned int data_size);

void lcd_init(void);

char lcd_xy(char c, char row, char col);

char lcd_clear_line(uint8_t row);

void lcd_clear(void);

void write_lcd_str(char*);

char code_to_char(uint8_t);

char poll_keyboard(void);
#endif
