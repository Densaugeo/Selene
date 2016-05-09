#include "Skirnir.hpp"
#include "base64.hpp"

Skirnir::Skirnir(HardwareSerial* port_) {
  port = port_;
  fsm_state = START;
  receive_buffer[60] = '\0';
}

void Skirnir::heartbeat() {
  port -> write("<\n");
}

void Skirnir::send45(uint8_t payload[]) {
  uint8_t send_buffer[63];
  
  send_buffer[0] = '#';
  encode_base64(payload, 45, send_buffer + 1);
  send_buffer[61] = '\n';
  send_buffer[62] = '\0';
  
  port -> write((char*) send_buffer);
}

void Skirnir::send(uint8_t payload[], uint32_t size) {
  int actual_size = size < 45 ? size : 45;
  uint8_t padded_payload[45];
  
  for(int i = 0; i < actual_size; ++i) {
    padded_payload[i] = payload[i];
  }
  
  for(int i = actual_size; i < 45; ++i) {
    padded_payload[i] = 0;
  }
  
  send45(padded_payload);
}

bool Skirnir::receive(uint8_t payload[], uint8_t next) {
  switch(next) {
    case '-':
      fsm_state = PING;
      break;
    case '#':
      fsm_state = PACKET_INTERMEDIATE;
      fsm_repeats = 0;
      break;
    default:
      switch(fsm_state) {
        case PING:
          if(next == '\n') port -> write(">\n");
          fsm_state = START;
          break;
        case PACKET_INTERMEDIATE:
          receive_buffer[fsm_repeats] = next;
          if(++fsm_repeats >= 60) fsm_state = PACKET_END;
          break;
        case PACKET_END:
          fsm_state = START;
          if(next == '\n') {
            decode_base64(receive_buffer, payload);
            return true;
          }
          break;
        default:
          break;
      }
  }
  
  return false;
}

bool Skirnir::receive_until_packet(uint8_t payload[]) {
  while(port -> available()) {
    if(receive(payload, port -> read())) {
      return true;
    }
  }
  
  return false;
}
