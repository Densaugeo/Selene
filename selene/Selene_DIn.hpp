/**
 * Represents a digital input Pin
 */
#ifndef SELENE_DIN_H_INCLUDED
#define SELENE_DIN_H_INCLUDED

#include <stdint.h>
#include "Selene_Pin.hpp"

namespace Selene {
  class DIn : public Pin {
    public:
      // Physical pin # on Arduino, which may not be the same as the Selene pin #
      const uint8_t arduinoPin;
      
      // True if pullup is used (default false)
      const bool pullup = false;
      
      // Constructor sets up physical pin
      DIn(uint8_t selenePin, uint8_t arduinoPin, bool pullup, uint8_t* info, uint8_t infoSize):
      Selene::Pin(selenePin, info, infoSize), arduinoPin(arduinoPin), pullup(pullup) {
        pinMode(arduinoPin, INPUT);
        digitalWrite(arduinoPin, pullup);
      }
      
      uint32_t getState() {
        return digitalRead(arduinoPin);
      }
      
      // State cannot be set
      void setState(uint32_t v) {}
  };
}

#endif // ifndef
