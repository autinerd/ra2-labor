/*
	Control a 7-segment display 
	at pins 8 to 14 of port A
*/

#include <xc.h>
#include "7segment.h"

typedef unsigned char u8;
typedef unsigned int u32;

// Define the LED digit patters, from 0 - 9
// 1 = LED on, 0 = LED off, in this order:
// Segment A, B, C, D, E, F, G

#define DECIMAL 0x80

uint8_t digits[16] = {
    0b0111111, // 0
    0b0000110, // 1
    0b1011011, // 2
    0b1001111, // 3
    0b1100110, // 4
    0b1101101, // 5
    0b1111101, // 6
    0b0000111, // 7
    0b1111111, // 8
    0b1101111, // 9
    0b1110111, // A
    0b1111100, // b
    0b0111001, // C
    0b1011110, // d
    0b1111001, // E
    0b1110001  // F
};
u8  seven_seg_digits[10][7] = { 
   { 1,1,1,1,1,1,0 },  // = 0
   { 0,1,1,0,0,0,0 },  // = 1
   { 1,1,0,1,1,0,1 },  // = 2
   { 1,1,1,1,0,0,1 },  // = 3
   { 0,1,1,0,0,1,1 },  // = 4
   { 1,0,1,1,0,1,1 },  // = 5
   { 1,0,1,1,1,1,1 },  // = 6
   { 1,1,1,0,0,0,0 },  // = 7
   { 1,1,1,1,1,1,1 },  // = 8
   { 1,1,1,0,0,1,1 }   // = 9
};

void initDisplay(void)
{               
    TRISACLR = 0b11111111 << 8; // A8 -15 -> output
    TRISBCLR = 0b101110; // B1-4 -> output
}

//void writeDisplay(u8 segment, u8 bit) {
//  if (bit == 1)
//    LATASET = ...  // must be completed 
//  else
//    LATACLR = ...  // must be completed
//}
// 
// void writeDot(u8 bit) {
//  if (bit == 1)
//    LATASET = ...  // must be completed 
//  else
//    LATACLR = ...  // must be completed
//}
   
//void sevenSegWrite(u8 digit) {
//   u8 segment;
//   for (segment = 0; segment < 7; segment++) {
//     writeDisplay(segment, seven_seg_digits[digit][segment]);
//   }
//}

void WriteDigit(uint8_t digit, int8_t number, uint8_t decimal)
{
    // If wrong digit number or wrong digit, return
    if ((digit > 3) || (number > 0xF) || (number < -1))
    {
        return;
    }
    // Clear digit
    LATACLR = (DECIMAL + digits[8]) << 8;
    // Clear all digit COM
    LATBCLR = 0b101110;
    // Select digit COM
    LATBSET = 1 << ((digit == 3) ? 5 : digit+1);
    // Set digit and decimal
    if (number != -1)
        LATASET = ((decimal ? DECIMAL : 0) + digits[number]) << 8;
}
