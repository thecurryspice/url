#ifndef definitions_h
#define definitions_h

#include <avr/io.h>
#include <avr/interrupt.h>

// #define DEBUG_PRINT
#define DEBUG_FPS
// #define DEBUG_ADC
// #define DEBUG_WAVE

#define ADC_INTERRUPT_MODE
// #define ADAPTIVE_REFERENCE
// #define FLASH_GATE

#define IND 13
#define INDON digitalWrite(IND, HIGH);
#define INDOFF digitalWrite(IND, LOW);
#define CHANNEL0 0
#define CHANNEL1 1
#define CHANNEL2 2
#define CHANNEL3 3
#define BASSPIN CHANNEL1
#define MIDPIN CHANNEL2
#define TREBLEPIN CHANNEL0
#define LDRPIN CHANNEL3
#define ANALOGREAD analogRead8bit
#define GETBASS ANALOGREAD(BASSPIN)
#define GETKICK ANALOGREAD(BASSPIN)>>2 ? ANALOGREAD(BASSPIN) : 0
#define GETTREBLE ANALOGREAD(TREBLEPIN)
#define GETMID ANALOGREAD(MIDPIN) - max(GETBASS, GETTREBLE)
#define GETEXTLIGHT ANALOGREAD(LDRPIN)
#define GETRANDOM(x,y) (uint8_t)random(x,y)

enum positions {FLASH, PULSE, USER, RANDOM, BASS, MID, TREBLE, LIGHT, TOTAL};
//enum positions {LIGHT, TREBLE, MID, BASS, USER, PULSE, FLASH, TOTAL};

#endif
