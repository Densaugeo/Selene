#include "Skirnir.hpp"

unsigned char packet_decoded[45];
unsigned long last_ping = 0;

Skirnir a_skirnir = Skirnir(&Serial);

int pb_previous = 0;
int pb_time = 0;
#define PB_DEBOUNCE 100
int virtual_pin = 0;
#define DISCOVERY 1
#define CONNECTION 2
#define DEVINFO 3
#define PININFO 4
#define PIN 5

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
    if(packet_decoded[0] == 'S') {
      if(packet_decoded[1] == 1 || packet_decoded[1] == 255) {
        switch(packet_decoded[5]) {
          case PIN:
            if(packet_decoded[6] == 0 && packet_decoded[7] & 0x80 && packet_decoded[11] <= 2) {
              digitalWrite(virtual_pin + 2, LOW);
              virtual_pin = packet_decoded[11];
              digitalWrite(virtual_pin + 2, HIGH);

              // Notify higher-level PC
              uint8_t pin_update[] = {'S', 1, 0, 0, 0, PIN, 0, 0, 0, 0, 0, virtual_pin, 0, 0, 0};
              a_skirnir.send(pin_update, 15);
            }
            break;
          case DISCOVERY:
            // Send devinfo packet
            uint8_t devinfo_packet[] = {'S', 1, 0, 0, 0, DEVINFO, 0, 0, 0, 0, 21, '{', '"', 'n', 'a', 'm', 'e', '"', ':', '"', 'S', 'e', 'l', 'e', 'n', 'e', ' ', 'O', 'n', 'e', '"', '}'};
            a_skirnir.send(devinfo_packet, 32);

            // And send a pininfo packet
            uint8_t pininfo_packet[] = {'S', 1, 0, 0, 0, PININFO, 0, 0, 0, 0, 31, '{', '"', 'n', 'a', 'm', 'e', '"', ':', '"', 'L', 'E', 'D', '#', '"', ',', '"', 'm', 'i', 'n', '"', ':', '0', ',', '"', 'm', 'a', 'x', '"', ':', '2', '}'};
            a_skirnir.send(pininfo_packet, 42);
            break;
        }
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
    uint8_t pin_update[] = {'S', 1, 0, 0, 0, PIN, 0, 0, 0, 0, 0, virtual_pin, 0, 0, 0};
    a_skirnir.send(pin_update, 15);
  }
}

