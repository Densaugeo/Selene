#include "Skirnir.hpp"
#include "Selene_Device.hpp"
#include "Selene_Pin.hpp"
#include "Adafruit_PWMServoDriver.h"
#include <EEPROM.h>

#define CURVATURE 4.0
#define PCA9685_ADDRESS 0x60

uint16_t last_ping = 0;

Skirnir a_skirnir = Skirnir(&Serial);

void send_with_skirnir(uint8_t payload[], uint8_t size) {
  a_skirnir.send(payload, size);
}

Adafruit_PWMServoDriver pca9685 = Adafruit_PWMServoDriver(PCA9685_ADDRESS);

const PROGMEM uint8_t pininfo_master[] = "{\"name\":\"Master\"}";
const PROGMEM uint8_t pininfo_red[] = "{\"name\":\"Red\"}";
const PROGMEM uint8_t pininfo_green[] = "{\"name\":\"Green\"}";
const PROGMEM uint8_t pininfo_blue[] = "{\"name\":\"Blue\"}";

class FancyPin : public Selene::Pin {
  private:
    uint16_t state = 0;
    const uint8_t eeprom;

  public:
    FancyPin(uint8_t selenePin, uint8_t eeprom, const uint8_t* info, uint8_t infoSize, bool infoProgmem):
      Selene::Pin(selenePin, info, infoSize, infoProgmem), eeprom(eeprom) {
        state = (uint16_t) EEPROM.read(eeprom + 1) << 8;
        state = state | EEPROM.read(eeprom);

        if(state > 4095) {
          state = 4095;
        }
    }

    uint32_t getState() {
      return state;
    }

    // Writing to physical pin is handled in custom Device subclass
    void setState(uint32_t v) {
      if (v <= 4095) {
        state = v;
      }
    }
    
    void saveState(uint32_t seleneAddress) {
      EEPROM.update(eeprom, state);
      EEPROM.update(eeprom + 1, state >> 8);
    }
};

class FancyDevice : public Selene::Device {
  private:
    Selene::Pin* internalPins[4];
    const uint8_t pcaPin;

  public:
    FancyDevice(uint32_t address, uint8_t pcaPin, const uint8_t* info, uint8_t infoSize, bool infoProgmem, void (*send)(uint8_t*, uint8_t)):
    Selene::Device(address, internalPins, 4, info, infoSize, infoProgmem, send), pcaPin(pcaPin) {
      pins[0] = new FancyPin(0, 8*(address - 1)    , pininfo_master, 17, true);
      pins[1] = new FancyPin(1, 8*(address - 1) + 2, pininfo_red, 14, true);
      pins[2] = new FancyPin(2, 8*(address - 1) + 4, pininfo_green, 16, true);
      pins[3] = new FancyPin(3, 8*(address - 1) + 6, pininfo_blue, 15, true);
    }

    void pinRequest(uint8_t pinNumber, uint32_t state) {
      if(pinNumber == 0) {
        pins[0] -> setState(state);
        updatePin(1);
        updatePin(2);
        updatePin(3);
      } else if(pinNumber < 4) {
        pins[pinNumber] -> setState(state);
        updatePin(pinNumber);
      }
    }

    void updatePin(uint8_t pinNumber) {
      uint32_t intermediate = (uint32_t) (pins[0] -> getState()) * (uint32_t) (pins[pinNumber] -> getState());
      pca9685.setPWM(pcaPin + pinNumber - 1, 0, intermediate / 4095);
    }

    // Not in constructor because this cannot be called until after the PCA9685 is set up
    void updatePhysicalPins() {
      updatePin(1);
      updatePin(2);
      updatePin(3);
    }
};

const PROGMEM  uint8_t devinfo_1[] = "{\"name\":\"Selene One\",\"desc\":\"Very first Selene device\",\"min\":0,\"max\":4095}";
const PROGMEM  uint8_t devinfo_2[] = "{\"name\":\"Corner Desk\",\"min\":0,\"max\":4095}";
const PROGMEM  uint8_t devinfo_3[] = "{\"name\":\"Torch\",\"min\":0,\"max\":4095}";

FancyDevice dev_1 = FancyDevice(1, 0, devinfo_1, 74, true, &send_with_skirnir);
FancyDevice dev_2 = FancyDevice(2, 3, devinfo_2, 41, true, &send_with_skirnir);
FancyDevice dev_3 = FancyDevice(3, 6, devinfo_3, 35, true, &send_with_skirnir);

// Stuff for reading slider pots
uint16_t last_pot_read = 0;
uint16_t last_address_change = 0;
uint8_t selected_address = 0;

// Scale pwm duties exponentially to counter logarithmic recency effect
// pwm out of 0..1 = (exp(duty*curvature) - 1)/exp(curvature)
double exp_curvature = exp(CURVATURE);
double recency_inverse(double duty) {
  return (exp(duty*CURVATURE) - 1.0)/exp_curvature;
}

void setup() {
  pca9685.begin();
  pca9685.setPWMFreq(1000);

  dev_1.updatePhysicalPins();
  delay(10);
  dev_2.updatePhysicalPins();
  delay(10);
  dev_3.updatePhysicalPins();

  Serial.begin(115200);
}

void loop() {
  uint16_t time = millis();

  // Start heartbeat every 2s
  if((uint16_t) time - last_ping > 2000) {
    last_ping = time;

    a_skirnir.heartbeat();
  }

  if(a_skirnir.receiveUntilPacket()) {
    dev_1.handlePacket(a_skirnir.receiveBuffer);
    dev_2.handlePacket(a_skirnir.receiveBuffer);
    dev_3.handlePacket(a_skirnir.receiveBuffer);
  }

  if(time - last_pot_read > 20) {
    last_pot_read = time;

    if(selected_address != analogRead(A3)*6/1023) {
      selected_address = analogRead(A3)*6/1023;
      last_address_change = time;
    }

    // Do not respond to pot change for 500ms after address change - user may still be setting addresss
    if(time - last_address_change > 500){
    if(selected_address <= 2) {
      FancyDevice* selected_device;

      switch(selected_address) {
        case 0: selected_device = &dev_1; break;
        case 1: selected_device = &dev_2; break;
        case 2: selected_device = &dev_3; break;
      }

      // Can't read alpha channel because there's no analog in for it yet
      selected_device -> pins[0] -> setState(4095);
      selected_device -> pins[1] -> setState(recency_inverse(analogRead(A0)/1023.0)*4095.0);
      selected_device -> pins[2] -> setState(recency_inverse(analogRead(A1)/1023.0)*4095.0);
      selected_device -> pins[3] -> setState(recency_inverse(analogRead(A2)/1023.0)*4095.0);

      selected_device -> updatePhysicalPins();
    }
    }
  }

  
  // Check if pins needs to send updates
  dev_1.sendPinUpdates(time);
  dev_2.sendPinUpdates(time);
  dev_3.sendPinUpdates(time);

  dev_1.savePinStates(time);
  dev_2.savePinStates(time);
  dev_3.savePinStates(time);
}
