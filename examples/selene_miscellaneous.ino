#include "Skirnir.hpp"
#include "Selene_Device.hpp"
#include "Selene_DIn.hpp"
#include "Selene_DOut.hpp"
#include "Selene_PWM.hpp"

//#include <EEPROM.h>
// This type of pin can be used to save pin states on EEPROM
/*class SaveReporter : public Selene::DOut {
  public:
    SaveReporter(uint8_t selenePin, uint8_t arduinoPin, uint8_t* info, uint8_t infoSize):
    DOut(selenePin, arduinoPin, info, infoSize) {
      setState(EEPROM.read(selenePin));
    }
    
    void saveState(uint32_t seleneAddress) {
      EEPROM.update(selenePin, getState());
    }
};*/


uint16_t last_ping = 0;

Skirnir a_skirnir = Skirnir(&Serial);

void send_with_skirnir(uint8_t payload[], uint8_t size) {
  a_skirnir.send(payload, size);
}

const PROGMEM  uint8_t devinfo_1[]  = "{\"name\":\"Selene One\",\"desc\":\"Very first Selene device\",\"min\":0,\"max\":255}";
const PROGMEM  uint8_t devinfo_2[]  = "{\"name\":\"PB + LED\",\"min\":0,\"max\":1}";

Selene::Pin* pins_1[3];
Selene::Pin* pins_2[2];

Selene::Device dev_1 = Selene::Device(1, pins_1, 3, devinfo_1, 73, true, &send_with_skirnir);
Selene::Device dev_2 = Selene::Device(2, pins_2, 2, devinfo_2, 35, true, &send_with_skirnir);

const PROGMEM uint8_t pin_1_0_info[] = "{\"name\":\"Red\"}";
const PROGMEM uint8_t pin_1_1_info[] = "{\"name\":\"Green\"}";
const PROGMEM uint8_t pin_1_2_info[] = "{\"name\":\"Blue\"}";
const PROGMEM uint8_t pin_2_7_info[] = "{\"name\":\"Pushbutton\"}";
const PROGMEM uint8_t pin_2_5_info[] = "{\"name\":\"Red LED\"}";

void setup() {
  dev_1.pins[0] = new Selene::PWM(0, 11, pin_1_0_info, 14, true);
  dev_1.pins[1] = new Selene::PWM(1, 10, pin_1_1_info, 16, true);
  dev_1.pins[2] = new Selene::PWM(2, 9, pin_1_2_info, 15, true);
  
  dev_2.pins[0] = new Selene::DIn(7, 7, true, pin_2_7_info, 21, true);
  dev_2.pins[1] = new Selene::DOut(5, 5, pin_2_5_info, 18, true);
  
  Serial.begin(115200);
}

void loop() {
  uint16_t time = millis();
  
  // Start heartbeat every 2s
  if(time - last_ping > 2000) {
    last_ping = time;
    
    a_skirnir.heartbeat();
  }
  
  // Returns true if a valid packet is found
  if(a_skirnir.receiveUntilPacket()) {
    dev_1.handlePacket(a_skirnir.receiveBuffer);
    dev_2.handlePacket(a_skirnir.receiveBuffer);
  }
  
  // Check if pins needs to send updates
  dev_1.sendPinUpdates(time);
  dev_2.sendPinUpdates(time);
  
  // To make a device with EEPROM pins save its state, use:
  //dev_1.savePinStates(millis());
  // (will only save if device has been unchanged for 60s, so save EEPROM writes
}
