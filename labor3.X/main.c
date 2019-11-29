/*
 First PIC32MM program
 
 This simple example program lets LED1 blink
 */

#include <stdint.h>
#include <xc.h>
#include <sys/attribs.h>
#include "sinus.h"
#include "notes.h"

#define CURRENT_TASK 3

Note melody[] = {
    {NOTE_C6, 4},
    {NOTE_G5, 8},
    {NOTE_G5, 8},
    {NOTE_A5, 4},
    {NOTE_G5, 4},
    {0, 4},
    {NOTE_B5, 4},
    {NOTE_C6, 4}
};

Note melody2[] = {
    {NOTE_C6, 8},
    {NOTE_D6, 8},
    {NOTE_E6, 8},
    {NOTE_F6, 8},
    {NOTE_G6, 4},
    {NOTE_G6, 4},
    {NOTE_A6, 8},
    {NOTE_A6, 8},
    {NOTE_A6, 8},
    {NOTE_A6, 8},
    {NOTE_G6, 2},
    {NOTE_A6, 8},
    {NOTE_A6, 8},
    {NOTE_A6, 8},
    {NOTE_A6, 8},
    {NOTE_G6, 2},
    {NOTE_F6, 8},
    {NOTE_F6, 8},
    {NOTE_F6, 8},
    {NOTE_F6, 8},
    {NOTE_E6, 4},
    {NOTE_E6, 4},
    {NOTE_D6, 8},
    {NOTE_D6, 8},
    {NOTE_D6, 8},
    {NOTE_D6, 8},
    {NOTE_C6, 2}
};
int8_t noteIndex = 0;

void NextOutput(void);
void SetFreqValue(void);
void setup(void);
void loop(void);

int main(void)
{
    setup();
    loop();
    return 0;
}

void setup(void)
{
    SYSTEM_Initialize();    // set 24 MHz clock for CPU and Peripheral Bus
                            // clock period = 41,667 ns = 0,0417 us
    
    // RB14 is DAC Output
    DAC1CONbits.DACOE = 1;  // DAC Output enable
    DAC1CONbits.ON = 1;     // Enable DAC
    DAC1CONbits.REFSEL = 3; // Ref Voltage = 3.3V
#if CURRENT_TASK < 3
    // RC8 (AN14) is Potentiometer input
    ANSELCbits.ANSC8 = 1;   // RC8 input
    TRISCbits.TRISC8 = 1;
    AD1CHSbits.CH0SA = 14;  // AN14 input
    AD1CON1bits.ON = 1;     // ADC on
    AD1CON1bits.MODE12 = 1; // 12-bit mode
    AD1CON1bits.SSRC = 7;   // Auto-convert
    AD1CON1bits.ASAM = 1;   // Auto-sample
    AD1CON2bits.SMPI = 15;  // Interrupt every 16th sample
    AD1CON3bits.ADCS = 255; // Fad = 24,000,000 Hz / 510 = 47058 Hz
    AD1CON3bits.SAMC = 31;  // Sample Freq = 47058 Hz / 31 = 1518 Hz
    IEC1bits.AD1IE = 1;     // ADC interrupt enable
    IPC8bits.AD1IP = 4;     // ADC priority 4
    IPC8bits.AD1IS = 1;
#endif
#if CURRENT_TASK == 1
    T1CONbits.TCKPS = 0;    // Timer 1 Prescaler 1
    PR1 = 1500;             // 24,000,000 Hz / 500 Hz / 32 Samples = 1500
                            // Highest freq: 24,000,000 Hz / 200 Hz / 32 Samples = 3750
                            // Lowest freq: 24,000,000 Hz / 1.000 Hz / 32 Samples = 750
    IPC4bits.T1IP = 5;      // Timer 1 Priority = 5
    IPC4bits.T1IS = 1;      // Timer 1 subpriority = 1
    IFS0bits.T1IF = 0;      // Clear interrupt bit
    IEC0bits.T1IE = 1;      // Enable Timer 1 interrupt
    T1CONbits.ON = 1;       // Starting Timer
#elif CURRENT_TASK >= 2
    T1CONbits.TCKPS = 0;    // Timer 1 Prescaler 1
    PR1 = 480;              // 24,000,000 Hz / 500 Hz / 100 Samples = 480
                            // Highest freq: 24,000,000 Hz / 200 Hz / 100 Samples = 1200
                            // Lowest freq: 24,000,000 Hz / 1.000 Hz / 100 Samples = 240
    IPC4bits.T1IP = 5;      // Timer 1 Priority = 3
    IPC4bits.T1IS = 1;      // Timer 1 subpriority = 1
    IFS0bits.T1IF = 0;      // Clear interrupt bit
    IEC0bits.T1IE = 1;      // Enable Timer 1 interrupt
    T1CONbits.ON = 1;       // Starting Timer
#endif
#if CURRENT_TASK == 3
    
    // Highest freq: 24,000,000 Hz / 4,978 Hz / 100 Samples = 502
    // Lowest freq: 24,000,000 Hz / 33 Hz / 100 Samples = 7272
    T2CONbits.TCKPS = 7;    // Timer 2 Prescaler 256
    PR2 = 5859;             // 24,000,000 Hz / 16 Hz / 256 = 5859
    IPC4bits.T2IP = 3;      //
    IPC4bits.T2IS = 1;
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
    T2CONbits.ON = 1;
#endif
}

void NextOutput(void)
{
#if CURRENT_TASK == 1
    DAC1CONbits.DACDAT = (DAC1CONbits.DACDAT + 1) % 32;
#elif CURRENT_TASK >= 2
    static uint8_t currentState = 0;
    asm volatile ("nop");
    asm volatile (
    "addi %[curState], %[curState], 1 \n" // currentState++
    "li $t0, 100 \n"                      // load 100 to $t0
    "bne %[curState], $t0, 1f \n"         // if currentState != 100 goto 1
    "or %[curState], $0, $0 \n"           // else currentState = 0; currentState % 100
    "1:"
    "addu $t0, %[sin5], %[curState] \n"   // $t0 = Address of sin5[currentState]
    "lbu $t0, 0($t0) \n"                  // $t0 = sin5[currentState]
    "lw $t1, %[dac] \n"                   // $t1 = DAC1CON
    "andi $t0, $t0, 0x1F \n"              // sin5[currentState] % 32
    "ins $t1, $t0, 16, 5 \n"                 // 
    "sw $t1, %[dac]"
    : [curState] "+r" (currentState), [dac] "=m" (DAC1CON)
    : [sin5] "r" (sin5_2)
    : "t0", "t1");
//    currentState = (currentState + 1) % 100;
//    DAC1CONbits.DACDAT = (sinus[currentState] >> 3) % 32;
#endif
}

#if CURRENT_TASK < 3
void SetFreqValue(void)
{
#if CURRENT_TASK == 1
    // Highest freq: PR1 = 750
    // Lowest freq:  PR1 = 3750
    // Range: 3750 - 750 = 3000 -> 12 bit
    // Full 12-bit range: 4096
    // Scale 4096 to 3000
    PR1 = (ADC1BUF0 * 3000 / 4096) + 750;
#elif CURRENT_TASK >= 2
    // Highest freq: PR1 = 240
    // Lowest freq:  PR1 = 1200
    // Range: 1200 - 240 = 960 -> 10 bit
    // Full 12-bit range: 4096
    // Scale 4096 to 960
    PR1 = (ADC1BUF0 * 960 / 4096) + 240;
#endif
}
#endif

#if CURRENT_TASK == 3
void setFreqValue(uint16_t freq)
{
    if (freq == 0)
        freq = 1;
    PR1 = 240000 / freq;
}
#endif

void __ISR(_TIMER_1_VECTOR, IPL5AUTO) Timer1Handler(void)
{
    NextOutput();
    IFS0bits.T1IF = 0; // clear interrupt bit
}

void __ISR(_TIMER_2_VECTOR, IPL3AUTO) Timer2Handler(void)
{
    static uint8_t currentDuration = 0;
    Note note = {0, 1};
    if (noteIndex >= 0)
    {
        note = melody2[noteIndex];
    }
    setFreqValue(note.freq);
    if (currentDuration != (16 / note.duration))
    {
        currentDuration++;
    }
    else
    {
        currentDuration = 0;
        noteIndex++;
    }
    if (noteIndex == (sizeof(melody2)/sizeof(melody2[0])))
    {
        noteIndex = -2;
    }
    IFS0bits.T2IF = 0; // clear interrupt bit
}

#if CURRENT_TASK < 3
void __ISR(_ADC_VECTOR, IPL4AUTO) ADCHandler(void)
{
    SetFreqValue();
    IFS1bits.AD1IF = 0;
}
#endif

void loop(void)
{
    while (1)
    {
        asm volatile ("nop");
    }
}