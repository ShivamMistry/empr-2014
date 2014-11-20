#ifndef _LOGGING_H
#define _LOGGING_H

// Serial code
#include "lpc17xx_uart.h"       // Central include files
#include "lpc17xx_pinsel.h"
#include "lpc_types.h"

void logging_init(void);

int write_usb_serial_blocking(char* buf, int length);

uint8_t strlen(char* buf);

int  write_string(char* buf);

int  write_line(char* buf);

void  write_int(unsigned int value);

int read_usb_serial_none_blocking(char* buf, int length);

#endif
