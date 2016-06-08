/**
 * Represents a PWM output Pin
 */
#ifndef SELENE_PWM_H_INCLUDED
#define SELENE_PWM_H_INCLUDED

#include <stdint.h>
#include "Selene_Pin.hpp"

namespace Selene {
  class PWM : public Pin {
    private:
      // State is what it sounds like
      uint8_t state = 0;
      
    public:
      // Physical pin # on Arduino, which may not be the same as the Selene pin #
      const uint8_t arduinoPin;
      
      // Constructor sets up physical pin
      PWM(uint8_t selenePin, uint8_t arduinoPin, const uint8_t* info, uint8_t infoSize, bool infoProgmem):
      Selene::Pin(selenePin, info, infoSize, infoProgmem), arduinoPin(arduinoPin) {
        pinMode(arduinoPin, OUTPUT);
        analogWrite(arduinoPin, 0);
      }
      
      uint32_t getState() {
        return state;
      }
      
      // Value of .state is mirrored to physical pin
      void setState(uint32_t v) {
        if(v <= 255) {
          state = v;
          analogWrite(arduinoPin, state);
        }
      }
  };
}

#endif // ifndef
