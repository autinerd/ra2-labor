#include "main.h"
#include "7segment.h"
#include "system.h"
#define CURRENT_TASK 2

volatile uint16_t tempRawValue = 0;
uint16_t tempValue = 0;
uint8_t tempScale = 0; // 0 = Celsius, 1 = Fahrenheit

void setup(void)
{
    SYSTEM_Initialize();    // set 24 MHz clock for CPU and Peripheral Bus
    initDisplay();                        // clock period = 41,667 ns = 0,0417 us
    ANSELAbits.ANSA0 = 1; // Input is RA0
    TRISAbits.TRISA0 = 1;
    AD1CHS = 0;
    AD1CON1bits.ON = 1;
    AD1CON1bits.MODE12 = 1;
    AD1CON1bits.SSRC = 7;
    AD1CON1bits.ASAM = 1;
    AD1CON2bits.SMPI = 31;
    AD1CON3bits.ADCS = 255;
    AD1CON3bits.SAMC = 31;
    IEC1bits.AD1IE = 1;
    IPC8bits.AD1IP = 2;
    IPC8bits.AD1IS = 1;
    AD1CON1bits.SAMP = 1;
    
    T1CONbits.TCKPS = 3;    // Timer 1 Prescaler 256
    PR1 = 94;            // 24,000,000 Hz / 1 kHz / 256 = 94
    IPC4bits.T1IP = 3;      // Timer 1 Priority = 3
    IPC4bits.T1IS = 1;      // Timer 1 subpriority = 1
    IFS0bits.T1IF = 0;      // Clear interrupt bit
    IEC0bits.T1IE = 1;      // Enable Timer 1 interrupt
    T1CONbits.ON = 1;
}

void __ISR(_ADC_VECTOR, IPL2AUTO) ADCHandler(void)
{
    // tempValue = (((ADC1BUF0 * 3300) / 1023) - 500) / 10;
    asm volatile (
    ".set at \n\t"
    "lw %0, ADC1BUF0 \n\t"
    "addi $t0, $zero, 3300 \n\t"
    "addi $t1, $zero, 102 \n\t"
    "multu %0, $t0 \n\t"
    "mflo %0 \n\t"
    "divu %0, $t1 \n\t"
    "mflo %0 \n\t"
    "addiu %0, %0, -50 \n\t"
    "lw $t0, IFS1 \n\t"
    "ori $t0, $t0, 2 \n\t"
    "sw $t0, IFS1 \n\t"
    ".set noat" : "=r" (tempValue): :  "t0", "t1"
    );
//    IFS1bits.AD1IF = 0;
}

void __ISR(_TIMER_1_VECTOR, IPL3AUTO) Timer1Handler(void)
{
    static uint16_t currentTemp = 0;
    static uint8_t currentDigit = 0;
    static uint16_t delayCounter = 0;
    currentDigit++;
    currentDigit %= 4;
    switch (currentDigit)
    {
        case 0:
            delayCounter++;
            if (delayCounter == 1000)
            {
                delayCounter = 0;
                currentTemp = tempValue;
            }
            WriteDigit(0, 0xC, 0);
            break;
        case 1:
            WriteDigit(1, (currentTemp % 10), 0);
            break;
        case 2:
            WriteDigit(2, (currentTemp % 100) / 10, 1);
            break;
        case 3:
            WriteDigit(3, (currentTemp % 1000) / 100, 0);
            break;       
    }
    IFS0bits.T1IF = 0; // clear interrupt bit
}

void Task2(void)
{
}

void Task1(void)
{
    static uint16_t number = 0;
    static uint8_t delay = 0;
    delay++;
    if (delay == 50)
    {
        delay = 0;
        number++;
        number %= 10000;
    }
    WriteDigit(0, number % 10, 0);
    delay_us(1000);
    WriteDigit(1, (number % 100) / 10, 0);
    delay_us(1000);
    WriteDigit(2, (number % 1000) / 100, 0);
    delay_us(1000);
    WriteDigit(3, (number % 10000) / 1000, 0);
    delay_us(1000);
}

void loop(void)
{
    while(1)
    {
//#if CURRENT_TASK == 1
//        Task1();
//#elif CURRENT_TASK == 2
        Nop();
//#endif
    }
}

int main(void)
{
    setup();
    loop();
}
