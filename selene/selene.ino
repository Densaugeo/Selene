#include "Skirnir.hpp"
#include "Selene_Device.hpp"
#include "Selene_AIn.hpp"
#include "Selene_DIn.hpp"
#include "Selene_DOut.hpp"
#include "Selene_PWM.hpp"

#include "Adafruit_PWMServoDriver.h"
#include "Selene_PCA9685.hpp"


//#include <EEPROM.h>

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


/*#define PB_DEBOUNCE 100

int pb_previous = 0;
int pb_time = 0;*/
uint8_t packet_decoded[45];
uint16_t last_ping = 0;
uint16_t last_analog = 0;

Skirnir a_skirnir = Skirnir(&Serial);

void send_with_skirnir(uint8_t payload[], uint8_t size) {
  a_skirnir.send(payload, size);
}

const PROGMEM  uint8_t devinfo_one[]  = "{\"name\":\"Selene One\",\"desc\":\"Very first Selene device\",\"min\":0,\"max\":255}";

Selene::Pin** pins = new Selene::Pin*[3];
Selene::Device a_device = Selene::Device(1, pins, 3, devinfo_one, 73, true, &send_with_skirnir);


const PROGMEM  uint8_t devinfo_two[]  = "{\"name\":\"PB + LED\",\"min\":0,\"max\":1}";

Selene::Pin** pins_two = new Selene::Pin*[2];
Selene::Device device_two = Selene::Device(2, pins_two, 2, devinfo_two, 35, true, &send_with_skirnir);

const PROGMEM  uint8_t devinfo_three[] = "{\"name\":\"Analog Pin\",\"desc\":\"Read Voltage (out of 5V)\"}";

Selene::Pin** pins_three = new Selene::Pin*[1];
Selene::Device device_three = Selene::Device(3, pins_three, 1, devinfo_three, 55, true, &send_with_skirnir);

#define PCA9685_ADDRESS 0x40
Adafruit_PWMServoDriver pca9685 = Adafruit_PWMServoDriver(PCA9685_ADDRESS);

const PROGMEM uint8_t pin_1_0_info[] = "{\"name\":\"Red\"}";
const PROGMEM uint8_t pin_1_1_info[] = "{\"name\":\"Green\"}";
const PROGMEM uint8_t pin_1_2_info[] = "{\"name\":\"Blue\"}";

const PROGMEM uint8_t pin_2_7_info[] = "{\"name\":\"Pushbutton\"}";
const PROGMEM uint8_t pin_2_5_info[] = "{\"name\":\"Red LED\"}";

const PROGMEM uint8_t pin_3_0_info[] = "{\"name\":\"V\",\"min\":0,\"max\":4095}";

void setup() {
  a_device.pins[0] = new Selene::PWM(0, 11, pin_1_0_info, 14, true);
  a_device.pins[1] = new Selene::PWM(1, 10, pin_1_1_info, 16, true);
  a_device.pins[2] = new Selene::PWM(2, 9, pin_1_2_info, 15, true);

  device_two.pins[0] = new Selene::DIn(7, 7, true, pin_2_7_info, 21, true);
  device_two.pins[1] = new Selene::DOut(5, 5, pin_2_5_info, 18, true);

  pca9685.begin();
  pca9685.setPWMFreq(1000);
  
  //device_three.pins[0] = new Selene::AIn(0, 5, (uint8_t*) "{\"name\":\"V\",\"min\":0,\"max\":1023}", 31, false);
  device_three.pins[0] = new Selene::PCA9685(0, &pca9685, 0, pin_3_0_info, 31, true);

  //pinMode(5, INPUT);
  //digitalWrite(5, HIGH); // Turn on pullup

  Serial.begin(115200);
}

void loop() {
  uint32_t time = millis();
  
  // Start heartbeat every 2s
  if((uint16_t) time - last_ping > 2000) {
    last_ping = time;

    a_skirnir.heartbeat();
  }

  // Returns true if a valid packet is found
  if(a_skirnir.receive_until_packet(packet_decoded)) {
    a_device.handlePacket(packet_decoded);
    device_two.handlePacket(packet_decoded);
    device_three.handlePacket(packet_decoded);
  }

  /*if(!digitalRead(5) == 0) {
    pb_previous = 0;
  } else if(pb_previous == 0 && millis() > pb_time + PB_DEBOUNCE) {
    pb_previous = 1;
    pb_time = millis();

    a_device.pins[0] -> setState(!a_device.pins[0] -> getState());
  }*/

  // Check if pins needs to send updates
  a_device.sendPinUpdates(time);
  device_two.sendPinUpdates(time);

  if((uint16_t) time - last_analog > 100) {
    last_analog = time;

    //device_three.sendAllPins = true;
    device_three.sendPinUpdates(time);
  }

  
  
  //a_device.savePinStates(millis());
}

