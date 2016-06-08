/**
 * Represents a analog input Pin
 */
#ifndef SELENE_AIN_H_INCLUDED
#define SELENE_AIN_H_INCLUDED

#include <stdint.h>
#include "Selene_Pin.hpp"

namespace Selene {
  class AIn : public Pin {
    public:
      // Physical pin # on Arduino, which may not be the same as the Selene pin #
      const uint8_t arduinoPin;
      
      // Constructor sets up physical pin
      AIn(uint8_t selenePin, uint8_t arduinoPin, const uint8_t* info, uint8_t infoSize, bool infoProgmem):
      Selene::Pin(selenePin, info, infoSize, infoProgmem), arduinoPin(arduinoPin) {}
      
      uint32_t getState() {
        return analogRead(arduinoPin);
      }
      
      // State cannot be set
      void setState(uint32_t v) {}
  };
}

#endif // ifndef
