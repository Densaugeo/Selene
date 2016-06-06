#include "Skirnir.hpp"
#include "base64.hpp"

Skirnir::Skirnir(HardwareSerial* port_) {
  port = port_;
  fsm_state = START;
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

void Skirnir::send180(uint8_t payload[]) {
  uint8_t send_buffer[243];
  
  send_buffer[0] = '&';
  encode_base64(payload, 180, send_buffer + 1);
  send_buffer[241] = '\n';
  send_buffer[242] = '\0';
  
  port -> write((char*) send_buffer);
}

void Skirnir::send(uint8_t payload[], uint32_t size) {
  if(size < 45) {
    uint8_t padded_payload[45];
    
    for(uint8_t i = 0; i < size; ++i) {
      padded_payload[i] = payload[i];
    }
    
    for(uint8_t i = size; i < 45; ++i) {
      padded_payload[i] = 0;
    }
    
    send45(padded_payload);
  } else if(size == 45) {
    send45(payload);
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

bool Skirnir::fsmGlobals(uint8_t payload[], uint8_t next, uint8_t input_buffer[]) {
  switch(next) {
    case '-':
      fsm_state = PING;
      return true;
    case '#':
      fsm_state = PACKET_INTERMEDIATE;
      fsm_repeats = 0;
      input_buffer[60] = '\0';
      return true;
    default:
      return false;
  }
}

uint8_t Skirnir::fsmLocals(uint8_t payload[], uint8_t next, uint8_t input_buffer[]) {
  switch(fsm_state) {
    case PING:
      if(next == '\n') port -> write(">\n");
      fsm_state = START;
      return 0;
    case PACKET_INTERMEDIATE:
      input_buffer[fsm_repeats] = next;
      if(++fsm_repeats >= 60) fsm_state = PACKET_END;
      return 0;
    case PACKET_END:
      fsm_state = START;
      if(next == '\n') {
        decode_base64(input_buffer, payload);
        return 45;
      }
      return 0;
    default:
      return 0;
  }
}

uint8_t Skirnir::receive(uint8_t payload[], uint8_t next) {
  if(fsmGlobals(payload, next, receive_buffer)) {
    return 0;
  } else {
    return fsmLocals(payload, next, receive_buffer);
  }
}

uint8_t Skirnir::receive_until_packet(uint8_t payload[]) {
  while(port -> available()) {
    if(receive(payload, port -> read())) {
      return 45;
    }
  }
  
  return 0;
}
