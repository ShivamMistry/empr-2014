#include "logging.h"
#include "i2c.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_pinsel.h"
#include "lpc_types.h"
#include "LPC17xx.h"
#include "stdio.h"

#define LCD_DISPLAY_ADDRESS 0x3B
#define KEYPAD_ADDRESS 0x21
#define LCD_CHAR(x) x + 0x80

static long long ms_ticks = 0;

void SysTick_Handler(void)
{
    // increment ms_ticks by one
    ms_ticks++;
}

void delay(long long time)
{
    long long start = ms_ticks;
    while ((ms_ticks - start) < time);
}

void stage1(void)
{
    write_line("Hello World");
}

void init(void)
{
    logging_init();
    SysTick_Config(SystemCoreClock / 1000);
    init_i2c();
    lcd_init();
}

void stage2(void)
{
    uint8_t data_out = 0x00;
    uint8_t data_in = 0x00;
    uint8_t count = 0;
    I2C_M_SETUP_Type i2c_m_setup;
    int i;
    for (i = 0; i < 127; i++) {
        i2c_m_setup.sl_addr7bit = i;
        i2c_m_setup.tx_data = &data_out;
        i2c_m_setup.tx_length = sizeof(data_out);
        i2c_m_setup.rx_data = &data_in;
        i2c_m_setup.rx_length = sizeof(data_in);
        i2c_m_setup.retransmissions_max = 3;
        if (I2C_MasterTransferData(LPC_I2C1, &i2c_m_setup, I2C_TRANSFER_POLLING)) {
            count++;
            // found a device
            char buffer[32];
            sprintf(buffer, "Found device at: 0x%x", i);
            write_line(buffer);
        }
    }
    char buffer[48];
    sprintf(buffer, "%d devices connected to i2c bus", count);
    write_line(buffer);
}


void stage3(void)
{
    lcd_clear();
    write_lcd_str("Hello World");
}

int log2(uint8_t n)
{
    int log = 0;
    while ((n >>= 1) != 0) log++;
    return log;
}

void stage4(void)
{
    // clear lcd
    lcd_clear();
    // set up keypad
    char str[34];
    for (int i = 0; i < 34; i++) str[i] = 0;
    uint8_t data_pos = 0;
    while (1) {
        char c = poll_keyboard();
        if (data_pos >= 32) { 
            data_pos = 0;
            lcd_clear();
            for (int i = 0; i < 34; i++) { str[i] = 0; }
        }
        str[data_pos++] = c;
        write_lcd_str(str);
        write_line(str);
        delay(250);        
    }
}

uint8_t char_to_digit(char c)
{
    switch(c) {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        default:
            return 0xFF;
    }
}

void stage5(void)
{
    // Clear the LCD
    lcd_clear();
    long long operand1=0;
    long long operand2=0;
    char last_op = 0;
    uint8_t curr_operand = 0;
    while (1) { //infinite loop
        // takes a char from the keypad input
        char c = poll_keyboard();
        int digit = char_to_digit(c);
        char op = 0;
        if (digit == 0xFF) {
            // A number wasn't pressed, so do a function
            if (c == 'A') {
                op = '+';
            }
            else if (c == 'B') {
                op = '-';
            }
            else if (c == 'C') {
                op = '*';
            }
            else if (c == 'D') {
                op = '/';
            }
            else if (c=='#') {
                op = '=';
            }
            else if (c == '*') {
                op = 'C';
            }
            if (last_op != 0) {
                // must mean that operand 1 and 2 were entered already
                long long result = 0;
                switch (last_op) {
                    case '+':
                        result = operand1 + operand2;
                        break;
                    case '-':
                        result = operand1 - operand2;
                        break;
                    case '*':
                        result = operand1 * operand2;
                        break;
                    case '/':
                        if (operand2 == 0)
                        {
                            write_lcd_str("NaN");
                            delay(250);
                            curr_operand = 0;
                            operand1=operand2=0;
                            op = last_op = 0;
                        } else {
                            result = operand1 / operand2;
                        }
                        break;
                    case '=':
                        result = operand1;
                        break;
                    default:
                        result = 0;
                        break;
                }
                if (op == 'C') {
                    lcd_clear();
                    curr_operand = 0;
                    operand1=0;
                    operand2=0;
                    op = 0;
                    last_op = 0;
                    delay(250);
                    continue;
                } else {
                    operand1 = result;
                    lcd_clear();
                    char buffer[32];
                    sprintf(buffer, "%lld", result);
                    write_lcd_str(buffer);
                    operand2 = 0;
                    curr_operand = 1;
                    last_op = op;
                }
            } else {
                curr_operand = 1;
                last_op = op;
            }
        } else {
            lcd_clear();
            if (curr_operand == 0) {
                operand1 = digit + (operand1 * 10);
                char buffer[32];
                sprintf(buffer, "%lld", operand1);
                write_lcd_str(buffer);
            } else {
                operand2 = digit + (operand2 * 10);
                char buffer[32];
                sprintf(buffer, "%lld", operand2);
                write_lcd_str(buffer);
            }
        }
        delay(250);
    }
}

int main(void)
{
    init();
    stage1();
    stage2();
    stage3();
    //stage4();
    stage5();
    while(1);
}

