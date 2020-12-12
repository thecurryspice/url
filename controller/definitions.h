#ifndef definitions_h
#define definitions_h

#include <avr/io.h>

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
#define EXTLIGHT ANALOGREAD(LDRPIN)

enum positions {LIGHT, TREBLE, MID, BASS, USER, PULSE, FLASH, TOTAL};

// Faster ADC handling.
// change analogRead8Bit to analogRead in lines 13, 14 and 15 to
// for more sensitivity
uint8_t analogRead8bit(uint8_t pin)
{
	uint8_t result;
	ADMUX |= (pin & 0x07);

  	// start the conversion
	ADCSRA |= (1<<ADSC);

	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC));

	//read only ADCH, 8 bits
	result = ADCH;
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
