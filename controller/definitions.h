#ifndef definitions_h
#define definitions_h

#include <avr/io.h>

#define IND 13
#define INDON digitalWrite(IND, HIGH);
#define INDOFF digitalWrite(IND, LOW);
#define BASSPIN A0
#define MIDPIN A1
#define TREBLEPIN A2
#define LDRPIN A3
#define GETBASS analogRead8bit(BASSPIN)
#define GETMID analogRead8bit(MIDPIN)
#define GETTREBLE analogRead8bit(TREBLEPIN)
#define EXTLIGHT analogRead(LDRPIN)

enum positions {LIGHT, TREBLE, MID, BASS, USER, PULSE, FLASH, TOTAL};

// Faster ADC handling.
// change analogRead8Bit to analogRead in lines 13, 14 and 15 to
// for more sensitivity
uint8_t analogRead8bit(uint8_t pin)
{
	uint8_t result;

	// set ADLAR high
	ADMUX |= _BV(ADLAR);

#if defined(ADMUX)
	ADMUX |= (pin & 0x07);
#endif

#if defined(ADCSRA) && defined(ADCL)
	// start the conversion
	ADCSRA |= _BV(ADSC);

	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC));

	//read only ADCH, 8 bits
	result = ADCH;
#else
	// we don't have an ADC, return 0
	result = 0;
#endif

	// combine the two bytes
	return result;
}

/*
------------
CONTROL BYTE
------------

1 byte sent for each colour

pos       7       6       5       4       3       2       1       0
route     0       0       0       0       0       0       0       0
                  F       P       L       U       B       M       T

F : Flash
L : Watch External Light Levels
P : Pulse (Breather Mode)
U : User
B : Bass
M : Mid
T : Treble


------------
CONTROL BYTE
------------

-------------------------------------------------------------------------

--------------
CONTROL PACKET
--------------

C<X><char>

Eg: ~CRB		(where B represents 66, hence Flashing and Mid routing)

--------------
CONTROL PACKET
--------------

-------------------------------------------------------------------------

------------
DATA PACKETS
------------

X = R/G/B

TIME 		:	T<X><char><char><char><char>
					
	where   :	<char>    <char>			16-bit total-value
			  multiplier  remainder		255*multiplier + remainder

USER 		:	U<X><char>

EXTLIGHT	:	E<X><char>


Eg:	~TGmXxM	==> 	onTime = 255*109 + 88	offTime = 255*120 + 77
	~URn
	~EB/

------------
DATA PACKETS
------------
*/

#endif