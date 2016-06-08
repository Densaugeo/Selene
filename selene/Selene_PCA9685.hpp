/**
 * Represents a PWM output Pin on a PCA9685 driver
 */
#ifndef SELENE_PCA9685_H_INCLUDED
#define SELENE_PCA9685_H_INCLUDED

#include <stdint.h>
#include "Selene_Pin.hpp"
#include "Adafruit_PWMServoDriver.h"

namespace Selene {
  class PCA9685 : public Pin {
    private:
      // State is what it sounds like
      uint16_t state = 0;
      
      Adafruit_PWMServoDriver* pcaChip;
    
    public:
      // Physical pin # on the PCA chip, which may not be the same as the Selene pin #
      const uint8_t pcaPin;
      
      // Constructor sets up physical pin
      PCA9685(uint8_t selenePin, Adafruit_PWMServoDriver* pcaChip, uint8_t pcaPin, const uint8_t* info, uint8_t infoSize, bool infoProgmem):
      Selene::Pin(selenePin, info, infoSize, infoProgmem), pcaChip(pcaChip), pcaPin(pcaPin) {
        pcaChip -> setPWM(pcaPin, 0, 0);
      }
      
      uint32_t getState() {
        return state;
      }
      
      // Value of .state is mirrored to physical pin
      void setState(uint32_t v) {
        if(v <= 4095) {
          state = v;
          pcaChip -> setPWM(pcaPin, 0, v);
        }
      }
  };
}

#endif // ifndef
