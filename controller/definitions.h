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
#define CHANNEL0 0
#define CHANNEL1 1
#define CHANNEL2 2
#define CHANNEL3 3
#define BASSPIN CHANNEL0
#define MIDPIN CHANNEL1
#define TREBLEPIN CHANNEL2
#define LDRPIN CHANNEL3
#define ANALOGREAD analogRead8bit
#define GETBASS ANALOGREAD(BASSPIN)
#define GETKICK ANALOGREAD(BASSPIN)>100 ? ANALOGREAD(BASSPIN)<<2 : 0
#define GETMID ANALOGREAD(MIDPIN)
#define GETTREBLE ANALOGREAD(TREBLEPIN)
#define GETEXTLIGHT ANALOGREAD(LDRPIN)
#define GETRANDOM(x,y) (uint8_t)random(x,y)

enum positions {RANDOM, FLASH, PULSE, USER, BASS, MID, TREBLE, LIGHT, TOTAL};
//enum positions {LIGHT, TREBLE, MID, BASS, USER, PULSE, FLASH, TOTAL};

#endif
