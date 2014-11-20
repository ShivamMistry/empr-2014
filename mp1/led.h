#include "lpc_types.h"
#include "lpc17xx_gpio.h"
#include "stdio.h"

#define ST_CTRL     (*((volatile unsigned long*)0xE000E010)) //page 783
#define ST_RELOAD   (*((volatile unsigned long*)0xE000E014))
#define ST_CURRENT  (*((volatile unsigned long*)0xE000E018))

#define uint unsigned int
#define ubyte unsigned char 

const uint leds[4] = {1 << 18, 1 << 20, 1 << 21, 1 << 23};
#define ALL leds[0] | leds[1] | leds[2] | leds[3]
void SysTickInit(void);
void delay(long long);
void display_digits(ubyte);
