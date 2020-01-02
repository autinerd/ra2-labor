/** 
 * Rechnerarchitekturen Labor - Benotete Aufgabe
 * @author Tobias Kühn (8626939), Sidney Kuyateh (1578641)
 */

#include <stdint.h>
#include <xc.h>
#include <sys/attribs.h>
#include "rgb.h"
#include "color_conv.h"


void setup(void);
void loop(void);
void display(void);
void InitSPI(void);
void InitTimer1(void);
void InitADC(void);

#define LEDS 24

rgbw pixels[LEDS] = {0,0,0,0};

#define MODE_SINGLE_LED_FADE 0
#define MODE_FULL_LED_FADE 1
#define MODE_CLOCK 2
#define COUNT_MODES 3

uint8_t mode = 0;

uint16_t speed = 100;

/** Milliseconds since reset
 * Maximum: 4.294.967.295 ms = 49.7 days
 */
volatile uint32_t time_millis = 0;

#define BIT_0 0xC0 // Bit 0: 0b1100 0000
#define BIT_1 0xF0 // Bit 1: 0b1111 0000

#define TWO_BITS(a,b) ((a) << 8) + (b)

#define S1Pressed() ((PORTB & 1 << 9) == 0)
#define S2Pressed() ((PORTC & 1 << 10) == 0)

/**
 * Mathematical modulo function
 * In C, (-x) mod n < 0, e.g. (-20) mod 3 = -2
 * In mathematics, (-20) mod 3 = 1
 */
#define mod(a,b) ((((a) % (b)) + (b)) % (b))

int main(void)
{
    setup();
    loop();
    return 0;
}

void display(void)
{
    uint16_t i = 0;
    asm volatile (
    ".set noreorder\n\t" // compiler shouldn't decide about branch delay slot
    "1:"
    "srl $t0, %[i], 3\n\t" // pixelindex = i >> 3
    "addu $t0, %[pixels], $t0\n\t" // Address of pixels[pixelindex]
    "lbu $t0, 0($t0)\n\t" // load pixels[pixelindex]
    "andi $t1, %[i], 7\n\t" // bitindex = i % 8
    "sub $t1, %[seven], $t1\n\t" // bitindex = 7 - (i % 8)
    "sra $t0, $t0, $t1\n\t" // pixels[pixelindex] >> bitindex
    "andi $t0, $t0, 3\n\t" // send_bits = (pixels[pixelindex] >> bitindex) & 3
    "bne $t0, %[two], 4f\n\t" // if (send_bits != 2) goto 1b
    "nop\n\t"
    "sw %[BIT_10],SPI1BUF\n\t" // Bits 10 in SPI1BUF
    "b 5f\n\t" // jump to SPI transmit check
    "addiu %[i], %[i], 2\n\t" // i += 2
    "2:"
    "sw %[BIT_11], SPI1BUF\n\t" // Bits 11 in SPI1BUF
    "b 5f\n\t" // jump to SPI transmit check
    "addiu %[i], %[i], 2\n\t" // i += 2
    "3:"
    "sw %[BIT_01], SPI1BUF\n\t" // Bits 01 in SPI1BUF
    "b 5f\n\t" // jump to SPI transmit check
    "addiu %[i], %[i], 2\n\t" // i += 2
    "4:"
    "beq $t0, %[three], 2b\n\t" // if send_bits == 3 goto 2b
    "nop\n\t"
    "beq $t0, %[one], 3b\n\t" // if send_bits == 1 goto 3b
    "nop\n\t"
    "sw %[BIT_00], SPI1BUF\n\t" // if send_bits == 0 send BIT_00
    "b 5f\n\t" // jump to continue
    "addiu %[i], %[i], 2\n\t" // i += 2
    "5:"
    "lw $t0, SPI1STAT\n\t" // load SPI1STAT
    "andi $t0, $t0, 8\n\t" // check SPI1STATbits.SPITBE, if transmit buffer is empty
    "beq $t0, $zero, 5b\n\t" // if transmit buffer is not empty, wait
    "nop\n\t"
    "bne %[i], %[num_bits], 1b\n\t" // if i != sizeof(pixels) * 8 continue for;
    "nop\n\t"
    : [i] "=r" (i)
    : [i2] "r" (i), // bug in GNU assembler, i has to be twice in argument list
      [num_bits] "r"(sizeof(pixels) * 8),
      [BIT_00] "r"(TWO_BITS(BIT_0, BIT_0)),
      [BIT_01] "r"(TWO_BITS(BIT_0, BIT_1)),
      [BIT_10] "r"(TWO_BITS(BIT_1, BIT_0)),
      [BIT_11] "r"(TWO_BITS(BIT_1, BIT_1)),
      [seven] "r"(7),
      [one] "r"(1),
      [two] "r"(2),
      [three] "r"(3),
      [pixels] "r" (pixels)
    : "t0", "t1"
    );
//    for (i = 0; i < sizeof(pixels) * 8; i += 2)
//    {
//        uint16_t pixelindex = i >> 3;
//        uint8_t bitindex = 7 - (i % 8);
//        uint8_t send_bits = (((uint8_t*)pixels)[pixelindex] >> bitindex) & 3;
//        switch (send_bits)
//        {
//            case 3:
//                SPI1BUF = TWO_BITS(BIT_1, BIT_1);
//                break;
//            case 2:
//                SPI1BUF = TWO_BITS(BIT_1, BIT_0);
//                break;
//            case 1:
//                SPI1BUF = TWO_BITS(BIT_0, BIT_1);
//                break;
//            case 0:
//                SPI1BUF = TWO_BITS(BIT_0, BIT_0);
//                break;
//        }
//        while (SPI1STATbits.SPITBE == 0) {asm volatile ("nop");};
//    }
}

void setup(void)
{
    SYSTEM_Initialize();    // set 24 MHz clock for CPU and Peripheral Bus
    
    // Button Configuration
    TRISBSET = (1 << 9);    // RB9: S1 (input)
    TRISCSET = (1 << 10);    // RC10: S2 (input)
    
    InitSPI();
    InitTimer1();
    InitADC();
}

void InitSPI(void)
{
    // SPI Configuration
    SPI1CONbits.MSTEN = 1;  // SPI Master
    SPI1CONbits.MODE16 = 1; // Send 16 bits (equals 2 NeoPixel bits)
    SPI1CONbits.DISSDI = 1; // Disconnect input pin
    SPI1BRG = 1;            // Baud Rate 24.000.000 / (2 * (1+1)) = 6.000.000 Baud
    SPI1CONbits.ON = 1;     // SPI on
}

void InitTimer1(void)
{
    // Timer 1 Configuration
    T1CONbits.TCKPS = 0;    // Timer 1 Prescaler 1
    PR1 = 24000;            // 24,000,000 Hz / 1,000 Hz = 24000
    
    IPC4bits.T1IP = 5;      // Timer 1 Priority = 5
    IPC4bits.T1IS = 1;      // Timer 1 subpriority = 1
    IFS0bits.T1IF = 0;      // Clear interrupt bit
    IEC0bits.T1IE = 1;      // Enable Timer 1 interrupt
    T1CONbits.ON = 1;       // Starting Timer
}

void InitADC(void)
{
    // RC8 (AN14) is Potentiometer input
    ANSELCbits.ANSC8 = 1;   // RC8 input
    TRISCbits.TRISC8 = 1;
    AD1CHSbits.CH0SA = 14;  // AN14 input
    AD1CON1bits.ON = 1;     // ADC on
    AD1CON1bits.MODE12 = 0; // 10-bit mode
    AD1CON1bits.SSRC = 7;   // Auto-convert
    AD1CON1bits.ASAM = 1;   // Auto-sample
    AD1CON2bits.SMPI = 1;  // Interrupt every sample
    AD1CON3bits.ADCS = 255; // Fad = 24,000,000 Hz / 510 = 47058 Hz
    AD1CON3bits.SAMC = 31;  // Sample Freq = 47058 Hz / 31 = 1518 Hz
    IEC1bits.AD1IE = 1;     // ADC interrupt enable
    IPC8bits.AD1IP = 4;     // ADC priority 4
    IPC8bits.AD1IS = 1;
}

void __ISR(_TIMER_1_VECTOR, IPL5AUTO) Timer1Handler(void)
{
    time_millis++; // increment milliseconds time
    if (time_millis % 10 == 0)
    {
        display(); // render every 10 ms
    }
    IFS0bits.T1IF = 0; // clear interrupt bit
}

void __ISR(_ADC_VECTOR, IPL4AUTO) ADCHandler(void)
{
    if (mode != MODE_CLOCK)
    {
        speed = ((1024 - ADC1BUF0) * 490 / 1023) + 10; // Scaling Potentiometer to speed -> 10 - 500 ms per step
    }
    IFS1bits.AD1IF = 0; // clear ADC interrupt bit
}

void loop(void)
{
    static uint8_t Button1State = 0; // S1 state
    static uint8_t Button2State = 0; // S2 state
    static uint8_t direction = 0; // 0 = forward, 1 = backward
    hsv hsv_color = {0, 255, 30}; // set up saturation and value
    uint8_t pos = 0; // current position
    int32_t last_update = 0; // last update time
    
    while (1)
    {
        if (S1Pressed() && !Button1State)
        {
            direction = (direction == 0) ? 1 : 0; // switch direction
            Button1State = 1;
        }
        else if (!S1Pressed() && Button1State)
        {
            Button1State = 0;
        }
        
        if (S2Pressed() && !Button2State)
        {
            mode = (mode + 1) % COUNT_MODES; // switch mode
            Button2State = 1;
        }
        else if (!S2Pressed() && Button2State)
        {
            Button2State = 0;
        }
        
        if ((time_millis - last_update) > speed) // next step
        {
            uint8_t i = 0;
            last_update = time_millis; // set current time as last update time
            if (mode == MODE_FULL_LED_FADE) // fade all LEDs
            {
                pos = mod((direction == 0) ? (pos - 1) : (pos + 1), LEDS);
                for (i = 0; i < LEDS; i++)
                {
                    pixels[i] = hsv2rgb((hsv){((i + pos) * HSV_HUE_RANGE / LEDS) % HSV_HUE_RANGE, hsv_color.sat, hsv_color.val});
                }
            }
            else if (mode == MODE_SINGLE_LED_FADE) // fade single LED
            {
                pos = mod((direction == 0) ? (pos + 1) : (pos - 1), LEDS);
                hsv_color.hue = (pos * (HSV_HUE_RANGE / LEDS)) % HSV_HUE_RANGE;
                for (i = 0; i < LEDS; i++)
                {
                    if (i == pos)
                    {
                        pixels[i] = hsv2rgb(hsv_color);
                    }
                    else
                    {
                        pixels[i] = (rgbw){0,0,0,0};
                    }
                }
            }
            else if (mode == MODE_CLOCK)
            {
                static uint8_t filled_leds = 1;
                uint8_t i = 0;
                speed = 1000 / (LEDS);
                for (i = 0; i < LEDS; i++)
                {
                    if (filled_leds > i)
                    {
                        pixels[i] = hsv2rgb((hsv){((i * HSV_HUE_RANGE / LEDS) % HSV_HUE_RANGE), hsv_color.sat, hsv_color.val});
                    }
                    else if (i == pos)
                    {
                        pixels[i] = hsv2rgb((hsv){((i * HSV_HUE_RANGE / LEDS) % HSV_HUE_RANGE), hsv_color.sat, hsv_color.val});
                    }
                    else
                    {
                        pixels[i] = (rgbw){0,0,0,0};
                    }
                }
                pos = mod((direction == 0) ? (pos - 1) : (pos - 1), LEDS);
                if (pos == filled_leds)
                {
                    filled_leds = (filled_leds + 1) % LEDS;
                }
            }
        }
    }
}