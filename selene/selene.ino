#include "Skirnir180.hpp"
#include "Selene_Device.hpp"
#include "Selene_DOut.hpp"

#define PB_DEBOUNCE 100

int pb_previous = 0;
int pb_time = 0;
unsigned char packet_decoded[45];
unsigned long last_ping = 0;

Skirnir180 a_skirnir = Skirnir180(&Serial);

void send_with_skirnir(uint8_t payload[], uint8_t size) {
  a_skirnir.send(payload, size);
}

Selene::Pin** pins = new Selene::Pin*[3];
Selene::Device a_device = Selene::Device(1, pins, 3, (uint8_t*) "{\"name\":\"Selene One\",\"description\":\"Very first Selene device\"}", 62, &send_with_skirnir);

void setup() {
  a_device.pins[0] = new Selene::DOut(0, 2, (uint8_t*) "{\"name\":\"Red\",\"min\":0,\"max\":1}", 30);
  a_device.pins[1] = new Selene::DOut(1, 3, (uint8_t*) "{\"name\":\"Green\",\"min\":0,\"max\":1}", 32);
  a_device.pins[2] = new Selene::DOut(2, 4, (uint8_t*) "{\"name\":\"Blue\",\"min\":0,\"max\":1}", 31);

  pinMode(5, INPUT);
  digitalWrite(5, HIGH); // Turn on pullup
  
  Serial.begin(115200);
}

void loop() {
  // Check timer for overflow
  if(millis() < pb_time) {
    pb_time = 0;
  }
  
  // Start heartbeat every 2s
  if(millis() > last_ping + 2000) {
    a_skirnir.heartbeat();
    last_ping = millis();
  } else if(millis() < last_ping) {
    // Reset after timer overflow
    last_ping = 0;
  }

  // Returns true if a valid packet is found
  if(a_skirnir.receive_until_packet(packet_decoded)) {
    a_device.handlePacket(packet_decoded);
  }

  if(!digitalRead(5) == 0) {
    pb_previous = 0;
  } else if(pb_previous == 0 && millis() > pb_time + PB_DEBOUNCE) {
    pb_previous = 1;
    pb_time = millis();

    a_device.pins[0] -> setState(!a_device.pins[0] -> getState());
  }

  // Check if pins needs to send updates
  a_device.sendPinUpdates();
}

