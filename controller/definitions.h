#ifndef definitions_h
#define definitions_h

#include <avr/io.h>

// #define DEBUG_PRINT

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

// Faster ADC handling.
// change analogRead8Bit to analogRead in lines 13, 14 and 15 to
// for more sensitivity
uint8_t analogRead8bit(uint8_t pin)
{
	ADMUX |= (pin & 0x07);

  	// start the conversion
	ADCSRA |= (1<<ADSC);

	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC));

	//read only ADCH, 8 bits
	return ADCH;
}

#endif
