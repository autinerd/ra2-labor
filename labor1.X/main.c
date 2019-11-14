/*
 First PIC32MM program
 
 This simple example program lets LED1 blink
 */

#include <stdint.h>
#include <xc.h>
#include <sys/attribs.h>

#define S1Pressed() ((PORTB & 1 << 9) == 0)
#define LedToggle(n) PORTAINV = (1 << ((n)+8))

#define CURRENT_TASK 0

void SYSTEM_Initialize(void);

void setup(void) { 
    SYSTEM_Initialize();    // set 24 MHz clock for CPU and Peripheral Bus
                            // clock period = 41,667 ns = 0,0417 us
    TRISBSET = (1 << 9);    // RB9: S1 (input)
    TRISACLR = (0x1F << 9); // RA9-13: LED1-5 (output)
    
#if CURRENT_TASK == 3
    T1CONbits.TCKPS = 3;    // Timer 1 Prescaler 256
    PR1 = 18750;            // 24,000,000 Hz / 5 Hz / 256 = 18750
    IPC4bits.T1IP = 3;      // Timer 1 Priority = 3
    IPC4bits.T1IS = 1;      // Timer 1 subpriority = 1
    IFS0bits.T1IF = 0;      // Clear interrupt bit
    IEC0bits.T1IE = 1;      // Enable Timer 1 interrupt
#elif CURRENT_TASK == 4
    
#endif
    
}

void Task2(void)
{
    static uint8_t ButtonState = 0;
    if (S1Pressed() && !ButtonState)
    {
        LedToggle(1);
        ButtonState = 1;
    }
    else if (!S1Pressed() && ButtonState)
    {
        ButtonState = 0;
    }
}

void __ISR(_TIMER_1_VECTOR, ipl3) Timer1Handler(void)
{
    LedToggle(1);
    IFS0bits.T1IF = 0; // clear interrupt bit
}

void loop(void) {
    while(1) {
        
  }
}

int main(void) {
    setup();
    loop();
}