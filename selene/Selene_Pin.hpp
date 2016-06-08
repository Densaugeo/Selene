/**
 * Abstract class for Selene Pins to inherit
 */
#ifndef SELENE_PIN_H_INCLUDED
#define SELENE_PIN_H_INCLUDED

#include <stdint.h>

namespace Selene {
  class Pin {
    public:
      // Selene Pin #
      const uint8_t selenePin;
      
      // Info for Selene pininfo packets
      const uint8_t* info;
      
      // Size of info, in bytes
      uint8_t infoSize;
      
      // True if .info points to a string in Arduino-style PROGMEM
      const bool infoProgmem;
      
      // Value of state on previous update sweep. Used by Selene::Device
      uint32_t statePrevious = 0;
      
      // Constructor just does initialization
      Pin(uint8_t selenePin, const uint8_t* info, uint8_t infoSize, bool infoProgmem) :
      selenePin(selenePin), info(info), infoSize(infoSize), infoProgmem(infoProgmem) {}
      
      // get/setstate to be implemented by inheriting class
      virtual uint32_t getState() = 0;
      virtual void setState(uint32_t v) = 0;
      
      /* saveState:
       *   Description:
       *     By default, .saveState() does nothing. An inheritor can override it to provide a function for
       *     saving state in EEPROM, which would normally be retrieved by the inheritor's constructor
       *   Parameters:
       *     seleneAddress - Selene address is provided (and selenePin property is available) to allow
       *       storing states in different memory areas based on address and pin #
       */
      virtual void saveState(uint32_t seleneAddress) {}
  };
}

#endif // ifndef
