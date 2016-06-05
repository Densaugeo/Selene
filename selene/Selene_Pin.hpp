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
      uint8_t* info;
      
      // Size of info, in bytes
      uint8_t infoSize;
      
      // Value of state on previous update sweep. Used by Selene::Device
      uint32_t statePrevious = 0;
      
      // Constructor just does initialization
      Pin(uint8_t selenePin, uint8_t* info, uint8_t infoSize) :
      selenePin(selenePin), info(info), infoSize(infoSize) {}
      
      // get/setstate to be implemented by inheriting class
      virtual uint32_t getState() = 0;
      virtual void setState(uint32_t v) = 0;
      
      // By default, .saveState() does nothing. An inheritor can override it to provide a function for
      // saving state in EEPROM, which would normally be retrieved by the inheritor's constructor
      virtual void saveState() {}
  };
}

#endif // ifndef
