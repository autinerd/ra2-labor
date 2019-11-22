/*
 First PIC32MM program
 
 This simple example program lets LED1 blink
 */

#include <stdint.h>
#include <xc.h>
#include <sys/attribs.h>

#define S1Pressed() ((PORTB & 1 << 9) == 0)
#define LedToggle(n) LATAINV = (1 << ((n)+9))

#define CURRENT_TASK 0

void SYSTEM_Initialize(void);

void setup(void) { 
    SYSTEM_Initialize();    // set 24 MHz clock for CPU and Peripheral Bus
                            // clock period = 41,667 ns = 0,0417 us
    TRISBSET = (1 << 9);    // RB9: S1 (input)
    TRISACLR = (0x1F << 10); // RA9-13: LED1-5 (output)
    TRISD &= 0b0111 ;     // set bit 3 of Port D for output
    TRISBbits.TRISB9 = 1;
#if CURRENT_TASK == 3
    T1CONbits.TCKPS = 3;    // Timer 1 Prescaler 256
    PR1 = 18750;            // 24,000,000 Hz / 1 kHz / 256 = 18750
    IPC4bits.T1IP = 3;      // Timer 1 Priority = 3
    IPC4bits.T1IS = 1;      // Timer 1 subpriority = 1
    IFS0bits.T1IF = 0;      // Clear interrupt bit
    IEC0bits.T1IE = 1;      // Enable Timer 1 interrupt
#elif CURRENT_TASK == 4
    
#endif
    
}

void Task1(void)
{
    int i;
    LATDSET = 0b1000;       // set bit 3 of Port D
	for (i=0; i< 1000000; i++);
        
    LATDCLR = 0b1000;       // clear bit 3 of Port D
    for (i=0; i< 1000000; i++); 
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

void Task3(void)
{
    
}

void __ISR(_TIMER_1_VECTOR, IPL3SOFT) Timer1Handler(void)
{
    LedToggle(1);
    IFS0bits.T1IF = 0; // clear interrupt bit
}

void loop(void) {
    while(1) {
#if CURRENT_TASK == 1
        Task1();
#elif CURRENT_TASK == 2
        Task2();
#elif CURRENT_TASK == 3
        Task3();
#elif CURRENT_TASK == 4
        Task4();
#elif CURRENT_TASK == 5
        Task5();
#endif
  }
}

int main(void) {
    setup();
    loop();
}