enum positions {TREBLE, MID, BASS, USER, LIGHT, PULSE, FLASH, TOTAL};

class LED
{
private:
    uint8_t _pin, _brightness[8] = {0,0,0,0,0,0,0,0};
    uint16_t _incrementStepTime = 0, _decrementStepTime = 0;
    uint16_t _onTime = 0, _offTime = 0;
    unsigned long _prevUpdate = 0;
    bool _state;
    byte control = 0b00000000;
    
    uint8_t calculateBrightness()
    {
        float totalBrightness = 0;
        uint8_t count = 0;
        // weighted values of all the individual brightness values make the total
        // a future update may have provisions for absolute value, but it isn't as aesthetic
        
        for (int i = 0; i < 7; i++)
        {
            if(control & (1<<i))
                count++;
        }
        //weight = count/255;

        for (int i = 0; i < 7; i++)
        {
            if(control & (1<<i))
                totalBrightness += float(_brightness[i]*count)/255;
        }

        // scale back to 255
        return totalBrightness*255;
    }

public:
    LED(uint8_t pin)
    {
        _pin = pin;
        _prevUpdate = millis();
        pinMode(_pin,OUTPUT);
    }

    void setBrightness(uint8_t brightness)
    {
        _brightness[0] = brightness;
        analogWrite(_pin,_brightness[0]);
    }

    uint8_t getBrightness()
    {
        // we don't want to call calculateBrightness for this
        // returns the current value of LED, which is the value after the last update
        return _brightness[TOTAL];
    }

    void setTime(uint16_t onTime, uint16_t offTime)
    {
        _onTime = onTime;
        _offTime = offTime;
    }

    void setTime(uint16_t Time)
    {
        _onTime = Time;
        _offTime = Time;
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
        _incrementStepTime = _onTime/256;
        _decrementStepTime = _offTime/256;
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
    }

    void update()
    {
        // flash
        if(control & (1<<FLASH))
        {
            if((_state) && (millis() - _prevUpdate) > _onTime)
            {
                _brightness[FLASH] = 0;
                _state = false;
                _prevUpdate = millis();
            }
            if((!_state) && (millis() - _prevUpdate) > _offTime)
            {
                _brightness[FLASH] = 255;
                _state = true;
                _prevUpdate = millis();
            }
        }

        //pulse
        if(control & (1<<PULSE))
        {
            // here state is used to check whether the brightness is increasing (0) or decreasing (1)
            // _state is explicitly set 'false' and brightness set to 0 before starting pulse
            if(!(_state) && (millis() - _prevUpdate) > _incrementStepTime)
            {
                if(_brightness[PULSE] == 255)
                    _state = true;
                else
                    _brightness[PULSE] += 1;
                _prevUpdate = millis();
            }
            if((_state) && (millis() - _prevUpdate) > _decrementStepTime)
            {
                if(_brightness[PULSE] == 0)
                    _state = false;
                else
                    _brightness[PULSE] -= 1;
                _prevUpdate = millis();
            }

        }

        _brightness[TOTAL] = calculateBrightness();
        analogWrite(_pin, _brightness[TOTAL]);
    }
};
