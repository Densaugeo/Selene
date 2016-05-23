#include "Skirnir180.hpp"
#include "base64.hpp"

Skirnir180::Skirnir180(HardwareSerial* port_) : Skirnir(port) {
  port = port_;
  fsm_state = START;
  receive_buffer[240] = '\0';
}

void Skirnir180::send180(uint8_t payload[]) {
  uint8_t send_buffer[243];
  
  send_buffer[0] = '&';
  encode_base64(payload, 180, send_buffer + 1);
  send_buffer[241] = '\n';
  send_buffer[242] = '\0';
  
  port -> write((char*) send_buffer);
}

void Skirnir180::send(uint8_t payload[], uint32_t size) {
  if(size <= 45) {
    return Skirnir::send(payload, size);
  } else if(size < 180) {
    uint8_t padded_payload[180];
    
    for(uint8_t i = 0; i < size; ++i) {
      padded_payload[i] = payload[i];
    }
    
    for(uint8_t i = size; i < 180; ++i) {
      padded_payload[i] = 0;
    }
    
    send180(padded_payload);
  } else {
    send180(payload);
  }
}

bool Skirnir180::fsmGlobals(uint8_t payload[], uint8_t next, uint8_t input_buffer[]) {
  switch(next) {
    case '&':
      fsm_state = LPACKET_INTERMEDIATE;
      fsm_repeats = 0;
      return true;
    default:
      return Skirnir::fsmGlobals(payload, next, input_buffer);
  }
}

uint8_t Skirnir180::fsmLocals(uint8_t payload[], uint8_t next, uint8_t input_buffer[]) {
  switch(fsm_state) {
    case LPACKET_INTERMEDIATE:
      input_buffer[fsm_repeats] = next;
      if(++fsm_repeats >= 240) fsm_state = LPACKET_END;
      return 0;
    case LPACKET_END:
      fsm_state = START;
      if(next == '\n') {
        decode_base64(input_buffer, payload);
        return 180;
      }
      return 0;
    default:
      return Skirnir::fsmLocals(payload, next, input_buffer);
  }
}

uint8_t Skirnir180::receive(uint8_t payload[], uint8_t next) {
  if(fsmGlobals(payload, next, receive_buffer)) {
    return 0;
  } else {
    return fsmLocals(payload, next, receive_buffer);
  }
}

uint8_t Skirnir180::receive_until_packet(uint8_t payload[]) {
  uint8_t result = 0;
  
  while(port -> available()) {
    result = receive(payload, port -> read());
    
    if(result) {
      return result;
    }
  }
  
  return result;
}
