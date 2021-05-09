#include "definitions.h"
#include "adcHandler.h"

#ifndef LED_H
#define LED_H

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
  #include "WConstants.h"
#endif

class LED
{
private:
    uint8_t _pin, _brightness[9] = {0,0,0,0,0,0,0,0,0}, _activeChannels = 0, _minBrightness = 0, _maxBrightness = 255, _minPulseBrightness = 0, _maxPulseBrightness = 255, _colourDepth = 0, _stepValue = 1;
    uint8_t _randomStep = 1, _randomJitter = 0;
    uint8_t bassIncrCount = 0, bassIncrCountThresh = 1, bassDecrCount = 0, bassDecrCountThresh = 1;
    uint16_t _incrementStepTime = 0, _decrementStepTime = 0;
    uint16_t _onTime = 0, _offTime = 0;
    unsigned long _prevUpdate = 0;
    bool _state, _invert;
    byte _control = 0b00000000;

public:
    LED(uint8_t pin)
    {
        _pin = pin;
        _prevUpdate = millis();
        pinMode(_pin,OUTPUT);
    }

    void setUserBrightness(uint8_t brightness)
    {
        _brightness[USER] = brightness;
    }

    void setBrightness(uint8_t brightness)
    {
        _brightness[TOTAL] = brightness;
        analogWrite(_pin,_brightness[TOTAL]);
    }

    void setColourDepth(uint8_t colourDepth)
    {
        _colourDepth = colourDepth;
        // have to add more things here
        _stepValue = 256/pow(2,_colourDepth);
        //we need to prepare variables again
        if(_control & (1<<PULSE))
            pulse(true);
        if(_control & (1<<FLASH))
            flash(true);
    }

    uint8_t getColourDepth()
    {
        return _colourDepth;
    }

    uint8_t getBrightness()
    {
        // we don't want to call calculateBrightness here,
        // instead, return the value of brightness after the last update
        return _brightness[TOTAL];
    }

    uint8_t getActiveChannels()
    {
        // return the number of channels that are routed to the LED
        uint8_t count = 0;
        for (int i = 0; i < 7; i++)
        {
            if(_control & (1<<i))
                count++;
        }
        return count;
    }

    byte getControlParameters()
    {
        // return the _control byte
        return _control;
    }

    bool setControlParameters(byte control)
    {
        if(control & (1<<PULSE) && control & (1<<FLASH))
            //flash and pulse should not be simultaneuosly on
            return false;
        else
        {
            (control & (1<<FLASH))  ? flash(true)           : flash(false);
            (control & (1<<PULSE))  ? pulse(true)           : pulse(false);
            (control & (1<<USER))   ? routeUser(true)       : routeUser(false);
            (control & (1<<BASS))   ? routeBass(true)       : routeBass(false);
            (control & (1<<MID))    ? routeMid(true)        : routeMid(false);
            (control & (1<<TREBLE)) ? routeTreble(true)     : routeTreble(false);
            (control & (1<<LIGHT))  ? watchExtLight(true)   : watchExtLight(false);
            (control & (1<<RANDOM)) ? invert(true)          : invert(false);
        }
        return true;
    }

    void avoid(uint8_t a, uint8_t t, uint8_t thresh) {}

    void setRandomStep(uint8_t randomStep, uint8_t randomJitter)
    {
        _randomStep = randomStep;
        _randomJitter = randomJitter;
    }

    void setTime(uint16_t onTime, uint16_t offTime)
    {
        // set the time variables for flashing and pulsing
        _onTime = onTime;
        _offTime = offTime;
    }

    void setTime(uint16_t Time)
    {
        // set the time variables for flashing and pulsing
        _onTime = Time;
        _offTime = Time;
    }

    // invert the signals for the LED
    void invert(bool invertColour)
    {
        _invert = invertColour;
    }

    // limits the maximum brightness, in a sense that
    // brightness is never constrained.
    // Eg: If pulse time is 256 ms and limit is 127,
    // each step increase will take 2 ms.
    void limit(uint8_t brightness)
    {
        // defaults to 0, which means no limit
        _maxBrightness = brightness;
        // update timing parameters
        if(_control & (1<<PULSE))
            pulse(true);
        if(_control & (1<<FLASH))
            flash(true);
    }

    void limit(uint8_t minBrightness, uint8_t maxBrightness)
    {
        // defaults to 0, which means no limit
        _maxBrightness = maxBrightness;
        // defaults to 0, which means no limit
        _minBrightness = minBrightness;

        // update timing parameters
        if(_control & (1<<FLASH))
            flash(true);
    }

    void limitPulseBrightness(uint8_t minPulseBrightness, uint8_t maxPulseBrightness)
    {
        // defaults to 0, which means no limit
        _maxPulseBrightness = maxPulseBrightness;
        // defaults to 0, which means no limit
        _minPulseBrightness = minPulseBrightness;

        // update timing parameters
        if(_control & (1<<PULSE))
            pulse(true);
    }

    void flash(bool flash)
    {
        if(!flash)
        {
            _control = _control & ~(1<<FLASH);
            // nothing more to do if flashing has been switched off
            return;
        }

        // switch off pulsing before flashing
        _control &= ~(1<<PULSE);
        _control |= (1<<FLASH);
        _state = false;

        _activeChannels = getActiveChannels();
    }

    void pulse(bool pulse)
    {
        if(!pulse)
        {
            _control = _control & ~(1<<PULSE);
            // nothing more to do if pulse has been switched off
            return;
        }

        // switch off flashing before pulsing
        _control &= ~(1<<FLASH);
        _control |= (1<<PULSE);
        _state = false;
        _brightness[PULSE] = 0;
        _incrementStepTime = (_onTime/(_maxPulseBrightness - _minPulseBrightness))*(_stepValue);
        _decrementStepTime = (_offTime/(_maxPulseBrightness - _minPulseBrightness))*(_stepValue);

        _activeChannels = getActiveChannels();
    }

    void routeRandom(bool argRandom)
    {
        if(!argRandom)
        {
            _control = _control & ~(1<<RANDOM);
            return;
        }

        _control |= (1<<RANDOM);

        _activeChannels = getActiveChannels();
    }

    void routeUser(bool user)
    {
        if(!user)
        {
            _control &= ~(1<<USER);
            // nothing more to do if user input is disabled
            return;
        }

        _control |= (1<<USER);

        _activeChannels = getActiveChannels();
    }

    void routeBass(bool bass)
    {
        if(!bass)
        {
            _control &= ~(1<<BASS);
            // nothing more to do if bass is not being routed
            return;
        }

        // set default breather mode off
        bassIncrCount = 0;
        bassDecrCount = 0;
        bassIncrCountThresh = 1;
        bassDecrCountThresh = 1;

        _control |= (1<<BASS);

        _activeChannels = getActiveChannels();
    }

    void routeBass(bool bass, uint8_t argIncrThresh, uint8_t argDecrThresh)
    {
        if(!bass)
        {
            _control &= ~(1<<BASS);
            // nothing more to do if bass is not being routed
            return;
        }

        // reset counters and update valuesfor thresholds
        bassIncrCount = 0;
        bassDecrCount = 0;
        bassIncrCountThresh = argIncrThresh;
        bassDecrCountThresh = argDecrThresh;

        _control |= (1<<BASS);

        _activeChannels = getActiveChannels();
    }

    void routeMid(bool mid)
    {
        if(!mid)
        {
            _control &= ~(1<<MID);
            // nothing more to do if mid is not being routed
            return;
        }

        _control |= (1<<MID);

        _activeChannels = getActiveChannels();
    }

    void routeTreble(bool treble)
    {
        if(!treble)
        {
            _control &= ~(1<<TREBLE);
            // nothing more to do if treble is not being routed
            return;
        }

        _control |= (1<<TREBLE);

        _activeChannels = getActiveChannels();
    }

    void watchExtLight(bool extlight, uint8_t brightness)
    {
        _brightness[LIGHT] = brightness;
        if(!extlight)
        {
            _control &= ~(1<<LIGHT);
            return;
        }
        _control |= (1<<LIGHT);
    }

    void watchExtLight(bool extlight)
    {
        if(!extlight)
        {
            _control &= ~(1<<LIGHT);
            return;
        }
        _control |= (1<<LIGHT);
    }

    void update()
    {
        // simple update : refresh total-brightness
        _brightness[TOTAL] = calculateBrightness();
        if(_invert)
            analogWrite(_pin, (0xFF)^(_brightness[TOTAL])); // 255 - _brightness[TOTAL])
        else
            analogWrite(_pin, _brightness[TOTAL]);
    }

    void update(uint8_t brightness)
    {
        // force update : specific brightness
        analogWrite(_pin, brightness);
    }


    uint8_t calculateBrightness()
    {
        // averages out all values of brightness
        // a future update will use specific weights to calculate the average

        // external light
        if(_control & (1<<LIGHT))
        {
            uint8_t value = GETEXTLIGHT;
            if(value > _brightness[LIGHT])
            {
                return 0;
            }
        }

        // flash
        if(_control & (1<<FLASH))
        {
            // here, state is used to check whether the LED is activated or not
            if((_state) && (millis() - _prevUpdate) > _onTime)
            {
                _state = false;
                _prevUpdate = millis();
                #ifndef FLASH_GATE
                    _brightness[FLASH] = _minBrightness;
                #else
                    return _minBrightness;
                #endif
            }
            if((!_state) && (millis() - _prevUpdate) > _offTime)
            {
                _state = true;
                _prevUpdate = millis();
                #ifndef FLASH_GATE
                    _brightness[FLASH] = _maxBrightness;
                #endif
            }

        }

        //pulse
        if(_control & (1<<PULSE))
        {
            // here, state is used to check whether the brightness is increasing (0) or decreasing (1)
            // _state is explicitly set 'false' and brightness set to 0 before starting pulse
            if(!(_state) && (millis() - _prevUpdate) > _incrementStepTime)
            {
                if(_brightness[PULSE] == _maxPulseBrightness)
                    _state = true;
                else
                    _brightness[PULSE] = constrain(_brightness[PULSE] + _stepValue, _minPulseBrightness, _maxPulseBrightness);
                _prevUpdate = millis();
            }
            if((_state) && (millis() - _prevUpdate) > _decrementStepTime)
            {
                if(_brightness[PULSE] == _minPulseBrightness)
                    _state = false;
                else
                    _brightness[PULSE] = constrain(_brightness[PULSE] - _stepValue, _minPulseBrightness, _maxPulseBrightness);
                _prevUpdate = millis();
            }

        }

        // bass routing
        if(_control & (1<<BASS))
        {
            // value is a 10 bit number, drop some bits to accomodate for set colour depth
            if(GETBASS > _brightness[BASS])
            {
                bassIncrCount++;
                if(bassIncrCount == bassIncrCountThresh)
                {
                    _brightness[BASS] = min(_brightness[BASS] + 1, 255);
                    bassIncrCount = 0;
                }
            }
            else
            {
                bassDecrCount++;
                if(bassDecrCount == bassDecrCountThresh)
                {
                    _brightness[BASS] = max(_brightness[BASS] - 1, 0);
                    bassDecrCount = 0;
                }
            }
        }

        // mid routing
        if(_control & (1<<MID))
        {
            // value is a 10 bit number, drop some bits to accomodate for set colour depth
            _brightness[MID] = GETMID;
        }

        // treble routing
        if(_control & (1<<TREBLE))
        {
            // value is a 10 bit number, drop some bits to accomodate for set colour depth
            _brightness[TREBLE] = GETTREBLE;
        }

        // random brightness gets applied on every update
        if(_control & (1<<RANDOM))
        {
            // slowly move towards the randomly defined value
            _brightness[RANDOM] = (GETRANDOM(0,_randomStep+1))*GETRANDOM(1,_randomJitter+1);
        }

        float totalBrightness = 0;
        // weighted values of all the individual brightness values make the total.
        // a future update may have provisions for absolute value or custom weights.
        // equal weighting is used for now:
        // weight = 255/_activeChannels;

        #ifndef FLASH_GATE
        for (int i = 0; i < 7; i++)
        #else
        for (int i = 1; i < 7; i++) // not including flash brightness
        #endif
        {
            if(_control & (1<<i))
                totalBrightness += float(_brightness[i])/_activeChannels;
                #ifdef DEBUG_PRINT
                    Serial.print(_brightness[i]);
                    Serial.print("\t");
                #endif
        }
        #ifdef DEBUG_PRINT
        Serial.println();
        #endif

        // if limits are different, constrain totalBrightness
        // this might be not required, will have to check the math here
        if(_maxBrightness != 255 || _minBrightness)
            return constrain(totalBrightness, _minBrightness, _maxBrightness);
        else
            return totalBrightness;
    }
};

#endif
