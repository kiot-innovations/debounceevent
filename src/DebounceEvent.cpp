/*

  Debounce buttons and trigger events
  Copyright (C) 2015-2018 by Xose Pérez <xose dot perez at gmail dot com>

  The DebounceEvent library is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  The DebounceEvent library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with the DebounceEvent library.  If not, see <http://www.gnu.org/licenses/>.


*/

#define TCA_SUPPORT  1

#define TCA6424A_OUTPUT             0
#define TCA6424A_INPUT              1

// #if TCA_SUPPORT ==  1
#include<Wire.h>
#include".\TCA6424.h"
TCA6424A TCAioExt;
// #endif
#include <Arduino.h>
#include ".\DebounceEvent.h"

// DebounceEvent::DebounceEvent(uint8_t pin, DEBOUNCE_EVENT_CALLBACK_SIGNATURE, uint8_t mode, unsigned long delay, unsigned long repeat) {
//     // this->callback = callback;
//     // _init(pin, mode, delay, repeat);
//     DebounceEvent(pin,DEBOUNCE_EVENT_CALLBACK_SIGNATURE,mode,delay,repeat,BUTTON_PROVIDER_ESP);
// }

DebounceEvent::DebounceEvent(uint8_t pin, DEBOUNCE_EVENT_CALLBACK_SIGNATURE, uint8_t mode, unsigned long delay, unsigned long repeat, uint8_t pinProvider) {
    this->callback = callback;
    _init(pin, mode, delay, repeat, pinProvider);
}

DebounceEvent::DebounceEvent(uint8_t pin, uint8_t mode, unsigned long delay, unsigned long repeat,uint8_t pinProvider) {
    this->callback = NULL;
    _init(pin, mode, delay, repeat,pinProvider);
}


// void DebounceEvent::_init(uint8_t pin, uint8_t mode, unsigned long delay, unsigned long repeat) {

//     // store configuration
//     _pin = pin;
//     _mode = mode & 0x01;
//     _defaultStatus = ((mode & BUTTON_DEFAULT_HIGH) > 0);
//     _delay = delay;
//     _repeat = repeat;

//     // set up button
//     #if ESP8266
//     if (_pin == 16) {
//         if (_defaultStatus) {
//             pinMode(_pin, INPUT);
//         } else {
//             pinMode(_pin, INPUT_PULLDOWN_16);
//         }
//     } else {
//     #endif // ESP8266
//         if ((mode & BUTTON_SET_PULLUP) > 0) {
//             pinMode(_pin, INPUT_PULLUP);
//         } else {
//             pinMode(_pin, INPUT);
//         }
//     #if ESP8266
//     }
//     #endif // ESP8266

//     _status = (_mode == BUTTON_SWITCH) ? digitalRead(_pin) : _defaultStatus;

// }
// new init
void DebounceEvent::_init(uint8_t pin, uint8_t mode, unsigned long delay, unsigned long repeat , uint8_t provider) {
    Serial.println("in lib  seriously i am here init  ");
    Serial.println(provider);
    // store configuration
    uint8_t test=5;
    
    Serial.println("adjsdvjds  "+String(test));
    (provider == BUTTON_PROVIDER_ESP)?(test=1):(test=0);
    Serial.println("adjsdvjds  "+String(test));
    _pin = pin;
    _mode = mode & 0x01;
    _defaultStatus = ((mode & BUTTON_DEFAULT_HIGH) > 0);
    _delay = delay;
    _repeat = repeat;
    _provider=provider;
    // set up button
    #if ESP8266
    if (_pin == 16) {
        if (_defaultStatus) {
            (provider == BUTTON_PROVIDER_ESP)?Serial.println("yaay its esp"):Serial.println("shitt its TCA");
            _pinMode(_pin, INPUT,_provider);
        } else {
            (provider == BUTTON_PROVIDER_ESP)?Serial.println("yaay its esp"):Serial.println("shitt its TCA");
            _pinMode(_pin, INPUT_PULLDOWN_16,_provider);
        }
    } else {
    #endif // ESP8266
        if ((mode & BUTTON_SET_PULLUP) > 0) {
            (provider == BUTTON_PROVIDER_ESP)?Serial.println("yaay its esp"):Serial.println("shitt its TCA");
            _pinMode(_pin, INPUT_PULLUP,_provider);
        } else {
            (provider == BUTTON_PROVIDER_ESP)?Serial.println("yaay its esp"):Serial.println("shitt its TCA");
            _pinMode(_pin, INPUT,_provider);
        }
    #if ESP8266
    }
    #endif // ESP8266

    _status = (_mode == BUTTON_SWITCH) ? _digitalRead(_pin,_provider) : _defaultStatus;

}

unsigned char DebounceEvent::loop() {

    unsigned char event = EVENT_NONE;

    if (_digitalRead(_pin,_provider) != _status) {

        // Debounce
        unsigned long start = millis();
        while (millis() - start < _delay) delay(1);

        if (_digitalRead(_pin,_provider) != _status) {

            _status = !_status;

            if (_mode == BUTTON_SWITCH) {

                event = EVENT_CHANGED;
                _event_start = millis();
                _ready = true;
                if (_reset_count) {
                    _event_count = 1;
                    _reset_count = false;
                } else {
                    ++_event_count;
                }

            } else {

                // released
                if (_status == _defaultStatus) {

                    _event_length = millis() - _event_start;
                    _ready = true;

                // pressed
                } else {

                    event = EVENT_PRESSED;
                    _event_start = millis();
                    _event_length = 0;
                    if (_reset_count) {
                        _event_count = 1;
                        _reset_count = false;
                    } else {
                        ++_event_count;
                    }
                    _ready = false;

                }

            }

        }
    }

    if (_ready && (millis() - _event_start > _repeat)) {
        _ready = false;
        _reset_count = true;
        if(_mode == BUTTON_SWITCH){
            event = EVENT_SWITCH_FINISHED;
        }else{
            event = EVENT_RELEASED;
        }
        
    }

    if (event != EVENT_NONE) {
        if (this->callback) this->callback(_pin, event, _event_count, _event_length);
    }

    return event;

}


void DebounceEvent::_pinMode(uint8_t pin,bool val,uint8_t provider){
    (provider == BUTTON_PROVIDER_ESP)?Serial.println("yaay its esp"):Serial.println("shitt its TCA");
    
    Serial.println("pinmode  prowider   "+String(provider));
    if(provider == BUTTON_PROVIDER_ESP){
        
        Serial.println("esp setting direction");

        Serial.println("hbfsj   " + String(BUTTON_PROVIDER_ESP));
        pinMode(pin,val);
    }
    else if (provider == 1){
        // #if TCA_SUPPORT ==  1
        
        TCAioExt.setPinDirection(pin,TCA6424A_INPUT);
        Serial.println("io extender setting direction");
        // delay(10000);
        // #endif
    }
       
}
bool DebounceEvent::_digitalRead(uint8_t pin,uint8_t provider){
    if(provider == BUTTON_PROVIDER_ESP){
        return _debounceRead(pin);
    }
    else if (provider == BUTTON_PROVIDER_TCA){
        // #if TCA_SUPPORT ==  1
        // Serial.println("io extender digital read");
        return TCAioExt.readPin(pin);
        // #endif
    }
}
bool DebounceEvent::_debounceRead(uint8_t pin,uint8_t provider){
    unsigned int ones = 0;
    unsigned int zeroes = 0;
    unsigned int debounceTime=5;
    unsigned long start = millis();
    while (millis() >= start && (millis() - start < debounceTime))
    {
        bool switch_status;
        switch_status = (provider==BUTTON_PROVIDER_ESP)?digitalRead(pin):TCAioExt.readPin(pin);
        switch_status ? ones++ : zeroes++;
    }
    return status ? ones > zeroes : zeroes > ones;
}