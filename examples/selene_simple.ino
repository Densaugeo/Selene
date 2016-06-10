#include "Skirnir.hpp"
#include "Selene_Device.hpp"
#include "Selene_DOut.hpp"

uint16_t last_ping = 0;

// Skirnir for handling and packetizing serial connection
Skirnir a_skirnir = Skirnir(&Serial);

// Sender function to pass into Selene devices
void send_with_skirnir(uint8_t payload[], uint8_t size) {
  a_skirnir.send(payload, size);
}

Selene::Pin* pins[1];
Selene::Device a_device = Selene::Device(1, pins, 1, (const uint8_t*) "{\"name\":\"Hello World!\",\"desc\":\"A simple example\"}", 49, false, &send_with_skirnir);

void setup() {
  a_device.pins[0] = new Selene::DOut(0, 13, (const uint8_t*) "{\"name\":\"Pin 13\",\"min\":0,\"max\":1}", 33, false);
  
  Serial.begin(115200);
}

void loop() {
  uint16_t time = millis();
  
  // Start heartbeat every 2s
  if(time - last_ping > 2000) {
    last_ping = time;

    a_skirnir.heartbeat();
  }

  // Check Skirnir for packet
  if(a_skirnir.receiveUntilPacket()) {
    // If found, pass to Selene device
    a_device.handlePacket(a_skirnir.receiveBuffer);
  }

  // Check if pins needs to send updates
  a_device.sendPinUpdates(time);
}
