/**
 * Represents a digital output Pin
 */
#ifndef SELENE_DOUT_H_INCLUDED
#define SELENE_DOUT_H_INCLUDED

#include "Selene_Pin.hpp"
#include <stdint.h>

namespace Selene {
  class DOut : public Pin {
    private:
      // State is what it sounds like
      bool state = 0;
      
    public:
      // Physical pin # on Arduino, which may not be the same as the Selene pin #
      const uint8_t arduinoPin;
      
      // Constructor sets up physical pin
      DOut(uint8_t selenePin, uint8_t arduinoPin, const uint8_t* info, uint8_t infoSize, bool infoProgmem):
      Selene::Pin(selenePin, info, infoSize, infoProgmem), arduinoPin(arduinoPin) {
        pinMode(arduinoPin, OUTPUT);
        digitalWrite(arduinoPin, LOW);
      }
      
      uint32_t getState() {
        return state;
      }
      
      // Value of .state is mirrored to physical pin
      void setState(uint32_t v) {
        if(v <= 1) {
          state = v;
          digitalWrite(arduinoPin, state);
        }
      }
  };
}

#endif // ifndef
