/*
 First PIC32MM program
 
 This simple example program lets LED1 blink
 */

#include <stdint.h>
#include <xc.h>
#include <sys/attribs.h>

#define S1Pressed() ((PORTB & 1 << 9) == 0)
#define LedToggle(n) LATAINV = (1 << ((n)+9))

#define CURRENT_TASK 5

uint8_t led_stat = 0;

void SYSTEM_Initialize(void);

void setup(void)
{ 
    SYSTEM_Initialize();    // set 24 MHz clock for CPU and Peripheral Bus
                            // clock period = 41,667 ns = 0,0417 us
    TRISBSET = (1 << 9);    // RB9: S1 (input)
    TRISACLR = (0x1F << 10); // RA10-14: LED1-5 (output)
    TRISD &= 0b0111;        // 
    
#if CURRENT_TASK == 3
    T1CONbits.TCKPS = 3;    // Timer 1 Prescaler 256
    PR1 = 9375;             // 24,000,000 Hz / 10 Hz / 256 = 9375
    IPC4bits.T1IP = 3;      // Timer 1 Priority = 3
    IPC4bits.T1IS = 1;      // Timer 1 subpriority = 1
    IFS0bits.T1IF = 0;      // Clear interrupt bit
    IEC0bits.T1IE = 1;      // Enable Timer 1 interrupt
    T1CONbits.ON = 1;       // Starting Timer
#elif CURRENT_TASK == 4
    T1CONbits.TCKPS = 3;    // Timer 1 Prescaler 256
    PR1 = 23438;            // 24,000,000 Hz / 4 Hz / 256 = 18750 ,5 aufrunden
    IPC4bits.T1IP = 3;      // Timer 1 Priority = 3
    IPC4bits.T1IS = 1;      // Timer 1 subpriority = 1
    IFS0bits.T1IF = 0;      // Clear interrupt bit
    IEC0bits.T1IE = 1;      // Enable Timer 1 interrupt
    T1CONbits.ON = 1;       // Starting Timer  
#elif CURRENT_TASK == 5
    CCP1CON1bits.MOD = 4;   // Dual Edge Compare Mode
    CCP1PR = 1000;          // 24MHz/1000= 24KHz
    CCP1CON1bits.ON = 1;    // PWM On
    CCP1RB = 1;           // example duty cicle 50%
    CCP1CON2bits.OCAEN = 1; // OCA enable
#endif
}

void Task1(void)
{
    int i;
    LATDSET = 0b1000;   // set bit 3 to Port D
    for (i=0; i<1000000; i++);
    
    LATDCLR = 0b1000;   //clear bit 3 of Port D
    for (i=0; i<1000000; i++);
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
    static uint8_t ButtonState = 0;
    if (S1Pressed() && !ButtonState)
    {
        led_stat ^= 1;
        ButtonState = 1;
    }
    else if (!S1Pressed() && ButtonState)
    {
        ButtonState = 0;
    }
}

void Task4(void)
{
    static uint8_t current_led = 0;        
    static uint8_t direction_led = 0;      //zero = increment, one = decrement
    if (direction_led == 0)
    {
        LedToggle(current_led);
        current_led++;
        LedToggle(current_led);
    }
    else
    {
        LedToggle(current_led);
        current_led--;
        LedToggle(current_led);
    }
    if (current_led == 5)
    {
        direction_led = 1;
    }
    else if (current_led == 1)
    {
        direction_led = 0;
    }
}

void Task5(void)
{
    static uint8_t fade_direction = 0;      // 0= up, 1=down
    
    if(fade_direction == 0)
    {
        CCP1RB += 10;
    }
    else if (fade_direction == 1)
    {
        CCP1RB -= 10;
    }
    if (CCP1RB >= 950)
    {
        fade_direction = 1;
    }
    else if (CCP1RB <= 10)
    {
        fade_direction = 0;
    }
    delay_us(50000);
}

void __ISR(_TIMER_1_VECTOR, ipl3soft) Timer1Handler(void)
{
#if CURRENT_TASK == 3
    if (led_stat == 1)
    {
        LedToggle(1);
    }
#elif CURRENT_TASK == 4
    Task4();
#endif
    IFS0bits.T1IF = 0; // clear interrupt bit
}

void loop(void) {
    while(1)
    {
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