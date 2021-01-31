#ifndef definitions_h
#define definitions_h

#include <avr/io.h>
#include <avr/interrupt.h>

// #define DEBUG_PRINT
#define DEBUG_FPS
// #define DEBUG_ADC

#define ADC_INTERRUPT_MODE

#define IND 13
#define INDON digitalWrite(IND, HIGH);
#define INDOFF digitalWrite(IND, LOW);
#define BASSPIN 0
#define MIDPIN 1
#define TREBLEPIN 2
#define LDRPIN 3
#define ANALOGREAD analogRead8bit
#define GETBASS ANALOGREAD(BASSPIN)
#define GETMID ANALOGREAD(MIDPIN)
#define GETTREBLE ANALOGREAD(TREBLEPIN)
#define GETEXTLIGHT ANALOGREAD(LDRPIN)
#define GETRANDOM(x,y) (uint8_t)random(x,y)

enum positions {RANDOM, FLASH, PULSE, USER, BASS, MID, TREBLE, LIGHT, TOTAL};
//enum positions {LIGHT, TREBLE, MID, BASS, USER, PULSE, FLASH, TOTAL};

#endif
