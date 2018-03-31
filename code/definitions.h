#define IND 13
#define INDON digitalWrite(IND, HIGH);
#define INDOFF digitalWrite(IND, LOW);
#define BASSPIN A0
#define MIDPIN A1
#define TREBLEPIN A2
#define LDRPIN A3
#define GETBASS analogRead(BASSPIN)
#define GETMID analogRead(MIDPIN)
#define GETTREBLE analogRead(TREBLEPIN)
#define EXTLIGHT analogRead(LDRPIN)

enum positions {LIGHT, TREBLE, MID, BASS, USER, PULSE, FLASH, TOTAL};

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


CONTROL PACKET : 3 x CONTROL BYTES
---------------------------------------------

------------
DATA PACKETS
------------

X = R/G/B

TIME 		:	<X><uint16_t><uint16_t>

USER 		:	<X><uint8_t>

EXTLIGHT	:	<X><uint8_t>

------------
DATA PACKETS
------------
*/