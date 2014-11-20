#include "logging.h"

// Read options
int read_usb_serial_none_blocking(char *buf,int length)
{
    return(UART_Receive((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)buf, length, NONE_BLOCKING));
}

uint8_t strlen(char* buf)
{
    char* ptr = buf;
    uint8_t size = 0;
    while(*ptr++ != '\0') size++;
    return size;
}

int write_string(char* buf)
{
    uint8_t size = strlen(buf);
    return write_usb_serial_blocking(buf, 1 + size); 
}

int write_line(char* buf)
{
    uint8_t size = strlen(buf);
    if (write_usb_serial_blocking(buf, 1 + size))
    {
        return write_usb_serial_blocking("\r\n", 3);
    }
    return 0;
   
}

int i2c(char** buffer, unsigned int value)
{
    unsigned int temp = value;
    unsigned int count = 0;
    if (value == 0)
    {
        (*buffer)[0] = '0';
        return 1;
    }
    while (temp > 0)
    {
        count++;
        int digit = temp % 10;
        char c = digit + '0';
        (*buffer)[count-1] = c;
        temp /= 10;
    }
    return count; 
}

void write_int(unsigned int value)
{
    char buffer[4];
    i2c(&buffer, value);
    write_string(buffer);
}
// Write options
int  write_usb_serial_blocking(char *buf,int length)
{
    return(UART_Send((LPC_UART_TypeDef *)LPC_UART0,(uint8_t *)buf,length, BLOCKING));
}
// init code for the USB serial line
void logging_init(void)
{
    UART_CFG_Type UARTConfigStruct;         // UART Configuration structure variable
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;    // UART FIFO configuration Struct variable
    PINSEL_CFG_Type PinCfg;             // Pin configuration for UART
    /*
     * Initialize UART pin connect
     */
    PinCfg.Funcnum = 1;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    // USB serial first
    PinCfg.Portnum = 0;
    PinCfg.Pinnum = 2;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 3;
    PINSEL_ConfigPin(&PinCfg);
        
    /* Initialize UART Configuration parameter structure to default state:
     * - Baudrate = 9600bps
     * - 8 data bit
     * - 1 Stop bit
     * - None parity
     */
    UART_ConfigStructInit(&UARTConfigStruct);
    /* Initialize FIFOConfigStruct to default state:
     * - FIFO_DMAMode = DISABLE
     * - FIFO_Level = UART_FIFO_TRGLEV0
     * - FIFO_ResetRxBuf = ENABLE
     * - FIFO_ResetTxBuf = ENABLE
     * - FIFO_State = ENABLE
     */
    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
    // Built the basic structures, lets start the devices/
    // USB serial
    UART_Init((LPC_UART_TypeDef *)LPC_UART0, &UARTConfigStruct);        // Initialize UART0 peripheral with given to corresponding parameter
    UART_FIFOConfig((LPC_UART_TypeDef *)LPC_UART0, &UARTFIFOConfigStruct);  // Initialize FIFO for UART0 peripheral
    UART_TxCmd((LPC_UART_TypeDef *)LPC_UART0, ENABLE);          // Enable UART Transmit
    
}
