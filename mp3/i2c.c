#include "i2c.h"
#include "logging.h"
#include "LPC17xx.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_pinsel.h"

void init_i2c(void)
{
    PINSEL_CFG_Type PinCfg; // declare data struct with param members
    PinCfg.OpenDrain = 0; // not in open drain mode
    PinCfg.Pinmode = 0; // on chip pull up resistor enabled
    PinCfg.Funcnum = 3;
    PinCfg.Pinnum = 0;
    PinCfg.Portnum = 0;
    PINSEL_ConfigPin(&PinCfg); // configure pin 0 of port0
    PinCfg.Pinnum = 1;
    PINSEL_ConfigPin(&PinCfg); // configure pin 1 of port0
    I2C_Init(LPC_I2C1, 100000); // Initialize I2C peripheral - by default devices are disabled on power-up
    I2C_Cmd(LPC_I2C1, ENABLE); // Enable I2C1 operation – 
}

Status write_i2c(uint8_t addr, void* data_out, unsigned int data_size)
{
    uint8_t data_in = 0x00;
    I2C_M_SETUP_Type i2c_m_setup;
    i2c_m_setup.sl_addr7bit = addr;
    i2c_m_setup.tx_data = data_out;
    i2c_m_setup.tx_length = data_size;
    i2c_m_setup.rx_data = &data_in;
    i2c_m_setup.rx_length = sizeof(data_in);
    i2c_m_setup.retransmissions_max = 3;
    return I2C_MasterTransferData(LPC_I2C1, &i2c_m_setup, I2C_TRANSFER_POLLING);

}

void lcd_init(void)
{
    char buf[12];
    buf[0]=0x00;
    buf[1]=0x34;
    buf[2]=0x0c;
    buf[3]=0x06;
    buf[4]=0x35;
    buf[5]=0x04;
    buf[6]=0x10;
    buf[7]=0x42;
    buf[8]=0x9f;
    buf[9]=0x34;
    buf[10]=0x80;
    buf[11]=0x02;
    write_i2c(LCD_DISPLAY_ADDRESS, buf, 12);    
}

char lcd_xy(char ddram_byte, char row, char column)
{
    char buf[0x10];
    char adr;
    if(row == 0)                                // line offset
        adr = column;
    else
        adr = 0x40 + column;
    buf[0] = 0x00;            // Enter function setting
    buf[1] = 0x80 + adr;    // LCD adr counter set to "adr"

    write_i2c(LCD_DISPLAY_ADDRESS,(char *)buf,2);

    buf[0] = 0x40;            // write to DDRAM
    buf[1] = ddram_byte;

    write_i2c(LCD_DISPLAY_ADDRESS,(char *)buf,2);

    return(0);
}


char lcd_clear_line(uint8_t row)
{
    char i=0, state;
    do
        //state = LCD_put_xy(LCD_ascii_to_lcd(0x20), row, i);
        state = lcd_xy(0xA0, row, i);
    while((i++<16));
    return state;
}


void lcd_clear(void)
{
    lcd_clear_line(0);                // clear 1st line
    lcd_clear_line(1);                // clear 2nd line
    return;
}

void write_lcd_str(char* buf)
{
    uint8_t length = strlen(buf);
    if (length > 32) {
        write_line("String too large for LCD");
        return;
    }
    char buffre[0x60];
    for (int i = 0; i < 0x60; i++) buffre[i] = LCD_CHAR(' ');
    buffre[0]=0x00;
    buffre[1]=0x80;
    // reposition the cursor
    write_i2c(LCD_DISPLAY_ADDRESS, buffre, 2);
    buffre[0] = 0x40;
        int i;
    uint8_t addr = 1;
    for (i = 0; i < length; i++) {
        if (addr == 17) addr = 41;
        buffre[addr++] = LCD_CHAR(buf[i]);
    }
    // string terminator
    buffre[addr+1] = 0x00;
    write_i2c(LCD_DISPLAY_ADDRESS, buffre, strlen(buffre));
    buffre[0]= 0x80;
    buffre[1]=0x02;
    write_i2c(LCD_DISPLAY_ADDRESS, buffre, 2);
}

char code_to_char(uint8_t code)
{
    if ((code & 0x88) == 0x88) return '1';
    if ((code & 0x48) == 0x48) return '2';
    if ((code & 0x28) == 0x28) return '3';
    if ((code & 0x18) == 0x18)  return 'A';
    if ((code & 0x84) == 0x84)  return '4';
    if ((code & 0x44) == 0x44)  return '5';
    if ((code & 0x24) == 0x24)  return '6';
    if ((code & 0x14) == 0x14)  return 'B';
    if ((code & 0x82) == 0x82)  return '7';
    if ((code & 0x42) == 0x42)  return '8';
    if ((code & 0x22) == 0x22)  return '9';
    if ((code & 0x12) == 0x12)  return 'C';
    if ((code & 0x81) == 0x81)  return '*';
    if ((code & 0x41) == 0x41)  return '0';
    if ((code & 0x21) == 0x21)  return '#';
    if ((code & 0x11) == 0x11)  return 'D';
    return ' ';
}

char poll_keyboard(void) 
{
    uint8_t data_in[0x60];
    uint8_t data_out[0x60];
    while(1) {
        data_out[0] = 0xf0;
        I2C_M_SETUP_Type i2c_m_setup;
        i2c_m_setup.sl_addr7bit = KEYPAD_ADDRESS;
        i2c_m_setup.tx_data = data_out;
        i2c_m_setup.tx_length = 1;
        i2c_m_setup.rx_data = data_in;
        i2c_m_setup.rx_length = 1;
        i2c_m_setup.retransmissions_max = 3;
        Status status = I2C_MasterTransferData(LPC_I2C1, &i2c_m_setup,
                I2C_TRANSFER_POLLING);
        // get the response out of the buffer
        uint8_t res=data_in[0];
        if (res != 0xf0) {
            // key was pressed
            // some col was selected, now find row
            data_out[0] = 0x0f;
            I2C_MasterTransferData(LPC_I2C1, &i2c_m_setup, I2C_TRANSFER_POLLING);
            uint8_t response_code = 0xFF ^ (res | data_in[0]);
            char c =  code_to_char(response_code);
            return c;
        } 
    }
}
