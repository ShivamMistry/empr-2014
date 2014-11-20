#include "led.h"
#include "LPC17xx.h"

long long ms_ticks = 0;

void SysTick_Handler(void)
{
    // increment ms_ticks by one
    ms_ticks++;
}

void init(void)
{
    SysTick_Config(SystemCoreClock / 1000);
}

void delay(long long time)
{
    long long start = ms_ticks;
    while ((ms_ticks - start) < time);
}

void stage1(void)
{
    for (int k = 0; k < 5; k++) {
        for (int led = 0; led < 4; led++) {
            GPIO_ClearValue(1, ALL);
            uint output = leds[led];
            GPIO_SetValue(1, output);
            delay(1000);
        }
    }
}


void display_digits(ubyte n)
{
    GPIO_ClearValue(1, ALL);
    for (int i = 0; i < 4; i++) {
        if (n & (1 << i)) {
            // check nth led 
            GPIO_SetValue(1, leds[i]);
        }
    }
}

void stage2()
{
    for (ubyte times = 0; times < 2; times++) {
        for (ubyte i = 0; i < 16; i++) {
            display_digits(i);   
            delay(1000); 
        }
    }
}

void main(void)
{
    init();
    GPIO_SetDir(1, ALL, 1);
    stage1();
    stage2();
}
