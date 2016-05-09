#include "Skirnir.hpp"

unsigned char packet_decoded[45];
unsigned long last_ping = 0;

Skirnir a_skirnir = Skirnir(&Serial);

int pb_previous = 0;
int pb_time = 0;
#define PB_DEBOUNCE 100
int virtual_pin = 0;
#define PINS 0
#define PINREQ 1
#define VALUE 0

void setup() {
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  
  pinMode(5, INPUT);
  digitalWrite(5, HIGH); // Turn on pullup
  
  Serial.begin(115200);
}

void loop() {
  // Check timer for overflow
  if(millis() < last_ping) {
    last_ping = 0;
  }
  if(millis() < pb_time) {
    pb_time = 0;
  }
  
  // Start heartbeat every 5s
  if(millis() > last_ping + 5000) {
    last_ping = millis();
    
    a_skirnir.heartbeat();
  }
  
  // Returns true if a valid packet is found
  if(a_skirnir.receive_until_packet(packet_decoded)) {
    if(packet_decoded[0] == 'S' && packet_decoded[1] == 1) {
      switch(packet_decoded[5]) {
        case PINREQ:
          if(packet_decoded[6] == 0 && packet_decoded[11] <= 2) {
            digitalWrite(virtual_pin + 2, LOW);
            virtual_pin = packet_decoded[11];
            digitalWrite(virtual_pin + 2, HIGH);

            // Notify higher-level PC
            uint8_t pin_update[] = {'S', 1, 0, 0, 0, PINS, 0, virtual_pin};
            a_skirnir.send(pin_update, 8);
          }
          break;
      }
    }
  }

  if(!digitalRead(5) == 0) {
    pb_previous = 0;
  } else if(pb_previous == 0 && millis() > pb_time + PB_DEBOUNCE) {
    pb_previous = 1;
    pb_time = millis();
    
    digitalWrite(virtual_pin + 2, LOW);
    virtual_pin = (virtual_pin + 1) % 3;
    digitalWrite(virtual_pin + 2, HIGH);

    // Notify higher-level PC
    uint8_t pin_update[] = {'S', 1, 0, 0, 0, PINS, 0, 0, 0, 0, 0, virtual_pin};
    a_skirnir.send(pin_update, 12);
  }
}
