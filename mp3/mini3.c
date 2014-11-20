#include "logging.h"
#include "i2c.h"

#include <stdio.h>
#include <math.h>

#include <LPC17xx.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_dac.h>
#include <lpc17xx_adc.h>

#define SIN_TABLE_SIZE 360
#define SIN(x) SIN_TABLE[x]
#define PI 3.141592654
#define SAMPLE_RATE 44100.0

static int16_t SIN_TABLE[SIN_TABLE_SIZE];
static uint64_t ts = 0; 

static float step_size = 0;
static float curr_pos = 0;

static int stage=0;

static uint8_t multiplier = 1;

void SysTick_Handler(void)
{
    ts++;
    if (stage != 2) {
        int16_t out = 0;
        uint16_t curr_pos_i = (uint16_t) curr_pos;
        float curr_pos_f = curr_pos - curr_pos_i;
        int16_t curr_sin = SIN(curr_pos_i);
        int16_t next_sin = SIN((curr_pos_i + 1) % SIN_TABLE_SIZE);
        out = curr_sin + ((next_sin - curr_sin) * curr_pos_f);
        curr_pos += step_size;
        if (curr_pos >= SIN_TABLE_SIZE) {
            curr_pos = curr_pos - SIN_TABLE_SIZE;
        }
        DAC_UpdateValue(LPC_DAC, (out * (multiplier/255.0)) + 512);
    }
}

void set_frequency(uint16_t freq)
{
    step_size = (SIN_TABLE_SIZE * freq) / SAMPLE_RATE;
}

void set_amplitude(uint8_t amplitude)
{
    multiplier = amplitude;
}

void dac_init(void)
{
    // Set up the pin configuration struct for using analogue out on mbed pin 18.
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum   = PINSEL_FUNC_2;
    PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
    PinCfg.Pinmode   = PINSEL_PINMODE_PULLUP;
    PinCfg.Portnum   = PINSEL_PORT_0;
    PinCfg.Pinnum    = PINSEL_PIN_26;
    // Configure mbed pin 18 to use the analogue out function.
    PINSEL_ConfigPin(&PinCfg);
    // Initialise the DAC so it is ready to use.
    DAC_Init(LPC_DAC);
    set_frequency(441);
    set_amplitude(255);
}

void adc_init(void)
{
    // Set up the pin configuration struct for using analogue in on mbed pin 16.
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum   = PINSEL_FUNC_1;
    PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
    PinCfg.Pinmode   = PINSEL_PINMODE_PULLUP;
    PinCfg.Portnum   = PINSEL_PORT_0;
    PinCfg.Pinnum    = PINSEL_PIN_24;
    
    // Configure mbed pin 16 to use the analogue in channel 1 function (AD0.1).
    PINSEL_ConfigPin(&PinCfg);
  
    // Enable ADC channel 1.
    ADC_Init(LPC_ADC, 1);
}

uint16_t read_adc(void)
{
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);
    ADC_StartCmd (LPC_ADC, ADC_START_NOW);
    while (ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_1, ADC_DATA_DONE) != SET);
    uint16_t result = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_1);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, DISABLE);
    return result;
}

void init(void)
{
    logging_init();
    init_i2c();
    for (size_t i = 0; i < SIN_TABLE_SIZE; i++) {
        SIN_TABLE[i] = sin(i * PI/180) * 512;
    }
    dac_init();
    adc_init();
    SysTick_Config(SystemCoreClock / SAMPLE_RATE);
}

void delay(long long time)
{
    time = (time * SAMPLE_RATE) / 1000;
    long start = ts;
    while ((ts - start) < time);
}

void stage2(void)
{
    while(1) {
        for (int i = 100; i < 5000; i += 100) {
            set_frequency(i);
            delay(1000);
        }
        set_frequency(441);
        for (int i = 100; i < 255; i++) {
            set_amplitude(i);
            delay(100);
        }
    }
}

void stage1(void)
{
    char buf[32];
    while (1)
    {
        uint16_t res = read_adc();
        sprintf(buf, "ADC Out: %d\n", res);
        write_line(buf);
        delay(1000);
    }
}

void stage3(void)
{
    stage = 2;
   // Set up the pin configuration struct for using analogue in on mbed pin 16.
   PINSEL_CFG_Type PinCfg;
   PinCfg.Funcnum   = PINSEL_FUNC_1;
   PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
   PinCfg.Pinmode   = PINSEL_PINMODE_PULLUP;
   PinCfg.Portnum   = PINSEL_PORT_0;
   PinCfg.Pinnum    = PINSEL_PIN_24;
   
   // Configure mbed pin 16 to use the analogue in channel 1 function (AD0.1).
   PINSEL_ConfigPin(&PinCfg);
   
   // Set up the ADC sampling at 200kHz (maximum rate).
   ADC_Init(LPC_ADC, SAMPLE_RATE);
   
   // Enable ADC channel 1.
   ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);
   
   // Set ADC to continuously sample.
   ADC_StartCmd (LPC_ADC, ADC_START_CONTINUOUS);
   
   // Set ADC to start converting.
   ADC_BurstCmd (LPC_ADC, ENABLE);
   
   // Enable interrupts for ADC conversion completing.
   NVIC_EnableIRQ(ADC_IRQn);
   
   // Enable interrupts globally.
   __enable_irq();
   while(1);
}


void ADC_IRQHandler (void)
{
   // Get the value read on ADC channel 1.
   uint16_t adc_value = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_1);
   //char buf[32];
   //sprintf(buf, "%d", adc_value);
   //write_line(buf);
   DAC_UpdateValue(LPC_DAC, adc_value >> 2);
}


int main(void)
{
    init();
    //stage1();
    //stage2();
    stage3();
    return 0;
}
