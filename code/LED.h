#include "definitions.h"

class LED
{
private:
    uint8_t _pin, _brightness[8] = {0,0,0,0,0,0,0,0}, activeChannels = 0, _maxBrightness = 255, _minBrightness = 0;
    uint16_t _incrementStepTime = 0, _decrementStepTime = 0;
    uint16_t _onTime = 0, _offTime = 0;
    unsigned long _prevUpdate = 0;
    bool _state;
    byte control = 0b00000000;    

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
            if(control & (1<<i))
                count++;
        }
        return count;
    }

    byte getControlParameters()
    {
        // return the control byte
        return control;
    }

    void avoid(uint8_t a, uint8_t t, uint8_t thresh) {}

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

    // limits the maximum brightness, in a sense that
    // brightness is never constrained.
    // Eg: If pulse time is 256 ms and limit is 127,
    // each step increase will take 2 ms.
    void limit(uint8_t brightness)
    {
        // defaults to 0, which means no limit
        _maxBrightness = brightness;
        // update timing parameters
        if(control & (1<<PULSE))
            pulse(true);
        if(control & (1<<FLASH))
            flash(true);        
    }

    void limit(uint8_t minBrightness, uint8_t maxBrightness)
    {
        // defaults to 0, which means no limit
        _maxBrightness = maxBrightness;
        // defaults to 0, which means no limit
        _minBrightness = minBrightness;

        // update timing parameters
        if(control & (1<<PULSE))
            pulse(true);
        if(control & (1<<FLASH))
            flash(true);
    }

    void flash(bool flash)
    {
        if(!flash)
        {
            control = control & ~(1<<FLASH);
            // nothing more to do if flashing has been switched off    
            return;
        }

        // switch off pulsing before flashing
        control &= ~(1<<PULSE);
        control |= (1<<FLASH);
        _state = false;
        
        activeChannels = getActiveChannels();
    }

    void pulse(bool pulse)
    {
        if(!pulse)
        {
            control = control & ~(1<<PULSE);
            // nothing more to do if pulse has been switched off
            return;
        }

        // switch off flashing before pulsing
        control &= ~(1<<FLASH);
        control |= (1<<PULSE);
        _state = false;
        _brightness[PULSE] = 0;
        _incrementStepTime = _onTime/(_maxBrightness - _minBrightness + 1);
        _decrementStepTime = _offTime/(_maxBrightness - _minBrightness + 1);
        
        activeChannels = getActiveChannels();
    }

    void routeUser(bool user)
    {
        if(!user)
        {
            control &= ~(1<<USER);
            // nothing more to do if user input is disabled  
            return;   
        }
        
        control |= (1<<USER);

        activeChannels = getActiveChannels();
    }

    void routeBass(bool bass)
    {
        if(!bass)
        {
            control &= ~(1<<BASS);
            // nothing more to do if bass is not being routed   
            return;   
        }
        
        control |= (1<<BASS);
        
        activeChannels = getActiveChannels();
    }

    void routeMid(bool mid)
    {
        if(!mid)
        {
            control &= ~(1<<MID);
            // nothing more to do if mid is not being routed   
            return;   
        }
        
        control |= (1<<MID);

        activeChannels = getActiveChannels();
    }

    void routeTreble(bool treble)
    {
        if(!treble)
        {
            control &= ~(1<<TREBLE);
            // nothing more to do if treble is not being routed   
            return;   
        }
        
        control |= (1<<TREBLE);

        activeChannels = getActiveChannels();
    }

    void watchExternalLight(bool extlight, uint8_t brightness)
    {
        _brightness[LIGHT] = brightness;
        if(!extlight)
        {
            control &= ~(1<<LIGHT);
            return;
        }
        control |= (1<<LIGHT);
    }

    void watchExternalLight(bool extlight)
    {
        if(!extlight)
        {
            control &= ~(1<<LIGHT);
            return;
        }
        control |= (1<<LIGHT);
    }

    void update()
    {
        // simple update : refresh total-brightness 
        _brightness[TOTAL] = calculateBrightness();
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
        if(control & (1<<LIGHT))
        {
            uint16_t value = EXTLIGHT;
            value = (value)>>2;
            if(value > _brightness[LIGHT])
            {
                digitalWrite(_pin, LOW);
                return;
            }
        }

        // flash
        if(control & (1<<FLASH))
        {
            // here, state is used to check whether the LED is activated or not
            if((_state) && (millis() - _prevUpdate) > _onTime/100)
            {
                _brightness[FLASH] = _minBrightness;
                _state = false;
                _prevUpdate = millis();
            }
            if((!_state) && (millis() - _prevUpdate) > _offTime/5)
            {
                _brightness[FLASH] = _maxBrightness;
                _state = true;
                _prevUpdate = millis();
            }
        }

        //pulse
        if(control & (1<<PULSE))
        {
            // here, state is used to check whether the brightness is increasing (0) or decreasing (1)
            // _state is explicitly set 'false' and brightness set to 0 before starting pulse
            if(!(_state) && (millis() - _prevUpdate) > _incrementStepTime)
            {
                if(_brightness[PULSE] == _maxBrightness)
                    _state = true;
                else
                    _brightness[PULSE] += 1;
                _prevUpdate = millis();
            }
            if((_state) && (millis() - _prevUpdate) > _decrementStepTime)
            {
                if(_brightness[PULSE] == _minBrightness)
                    _state = false;
                else
                    _brightness[PULSE] -= 1;
                _prevUpdate = millis();
            }

        }

        // bass routing
        if(control & (1<<BASS))
        {
            uint16_t value = GETBASS;
            // value is a 10 bit number, drop 2 to accomodate for uint8_t
            _brightness[BASS] = (value)>>2;
        }

        // mid routing
        if(control & (1<<MID))
        {
            uint16_t value = GETMID;
            // value is a 10 bit number, drop 2 to accomodate for uint8_t
            _brightness[MID] = (value)>>2;
        }

        // treble routing
        if(control & (1<<TREBLE))
        {
            uint16_t value = GETTREBLE;
            // value is a 10 bit number, drop 2 to accomodate for uint8_t
            _brightness[TREBLE] = (value)>>2;
        }

        float totalBrightness = 0;
        // weighted values of all the individual brightness values make the total.
        // a future update may have provisions for absolute value or custom weights.
        // equal weighting is used for now:
        // weight = 255/activeChannels;

        for (int i = 1; i < 7; i++)
        {
            if(control & (1<<i))
                totalBrightness += float(_brightness[i])/activeChannels;
        }

        // if limits are different, constrain totalBrightness
        // this might be not required, will have to check the math here
        if(_maxBrightness != 255 || _minBrightness)
            return constrain(totalBrightness, _minBrightness, _maxBrightness);
        else
            return totalBrightness;
    }
};