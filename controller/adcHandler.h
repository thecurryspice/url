#ifndef adchandler_h
#define adchandler_h

#include "definitions.h"

#ifdef ADC_INTERRUPT_MODE
// volatile variables for storing values
// from different channels in interrupted mode
volatile uint8_t channelValue[4] = {0};
#ifdef ADAPTIVE_REFERENCE
    static uint8_t channelOffset[4] = {0, 0, 0, 0};
#else
    static uint8_t channelOffset[4] = {22, 16, 20, 5};
    static uint8_t channelTrim[4] = {6, 32, 6, 6};
#endif
volatile uint8_t channelIndex = 0;
volatile uint32_t check = 0;

ISR(ADC_vect)
{
    // switch based on the channelIndex
    switch (channelIndex)
    {
        case 0 : channelValue[0] = ADCH; break;
        case 1 : channelValue[1] = ADCH; break;
        case 2 : channelValue[2] = ADCH; break;
        case 3 : channelValue[3] = ADCH; break;
        default: break;
    }
    // switch to the next channel
    channelIndex++;
    if(channelIndex > 3)
        channelIndex = 0;

    ADMUX &= ~(0x0F);               // clear previous channel index (last 4 bits)
    ADMUX |= (channelIndex & 0x07); // select next channel

    // fire the next reading
    ADCSRA |= (1 << ADSC);
    check++;
}
#endif

// initialise the ADC.
void initADC()
{
    #ifdef ADC_INTERRUPT_MODE
    // Serial.println("Initiated in interrupt mode");
    ADCSRA |= (1 << ADIE);  // enable interrupt
    #endif
    ADCSRA |= (1 << ADEN);  // enable ADC
    ADCSRA |= (1 << ADSC);  // kick off first reading
    // ADSC can't be set before ADEN is set called
}

// stop the ADC.
// Must be called if there is no colour channel using any analog channel.
// Otherwise, the processor will spend time in unnecessary interrupts.
void stopADC()
{
    #ifdef ADC_INTERRUPT_MODE
    ADCSRA &= ~(1 << ADIE);  // disable interrupt
    #endif
    ADCSRA &= ~(1 << ADEN);  // disable ADC
}

// setup the ADC with focus on faster AD conversions
//  - ADLAR
//  - prescalar of 16
//
// Turns off the ADC - needs an explicit initADC() call before using it
void setupADC()
{
    stopADC();
    #ifdef ADAPTIVE_REFERENCE
        ADMUX &= ~(1 << REFS0 | 1 << REFS1);    // set AREF pin as reference
    #else
        ADMUX |= (1 << REFS0);  // set default reference voltage
    #endif
    ADMUX |= (1 << ADLAR);  // left align ADC value to 8 bits from ADCH register

    // There's no need to spam the processor with ADC interrupts.
    // For a prescalar of 128, ADC frequency is 125 KHz
    // 125/13 = 9.6 KHz, which divided across 4 analog channels is 2400 Hz
    // For a desired FPS of 240 Hz, the ADC is still sampling 9 times extra
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);   // 128 prescalar for 125 KHz
    // ADCSRA |= (1 << ADPS2);                                 // 16 prescaler for 1 MHz
    // ADCSRA |= (1 << ADPS2) | (1 << ADPS0);                  // 32 prescaler for 500 KHz
}

// Faster ADC handling.
// change analogRead8Bit to analogRead in lines 13, 14 and 15 to
// for more sensitivity
uint8_t analogRead8bit(uint8_t pin)
{
    #ifdef ADC_INTERRUPT_MODE
        #ifdef DEBUG_ADC
        Serial.print(F("Pin:\t"));
        Serial.print(pin);
        Serial.print(F("\tADCI: \t"));
        Serial.println(channelValue[pin]);
        #endif
        // get the last updated channelValue
        return (channelValue[pin]>(channelOffset[pin]+channelTrim[pin])) ? channelValue[pin]-channelOffset[pin] : 0;
    #else
        ADMUX &= ~(0x0F);       // clear previous channel index (last 4 bits)
        ADMUX |= (pin & 0x07);  // set new channel index
        // start the conversion
    	ADCSRA |= (1 << ADSC);

    	// ADSC is cleared when the conversion finishes
    	while (bit_is_set(ADCSRA, ADSC));

        #ifdef DEBUG_ADC
        Serial.print(F("ADC:\t"));
        Serial.print(ADCH);
        Serial.print(F("\t on pin \t"));
        Serial.println(pin);
        #endif

    	//read only ADCH, 8 bits
    	return ADCH;
    #endif
}

void calibrateBass()
{
    #ifdef DEBUG_ADC
    Serial.println(F("Calibrating Bass input"));
    #endif
    channelOffset[BASSPIN] = 0;
    uint16_t average = 0;
    for (uint8_t i = 0; i < 16; i++)
    {
        average += analogRead8bit(BASSPIN);
        delay(100);
    }
    average = average >> 4;
    channelOffset[BASSPIN] = average;
    #ifdef DEBUG_ADC
    Serial.print("Calibrated Zero at: ");
    Serial.println(average);
    delay(1000);
    #endif
}

#endif
