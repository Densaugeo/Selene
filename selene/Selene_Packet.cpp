#include "Selene_Packet.hpp"

#include "string.h"

namespace Selene {
  bool Packet::type_has_pin(uint8_t type) {
    switch(type) {
      case 4: return true;
      case 5: return true;
      default: return false;
    }
  }
  
  uint8_t Packet::type_p_size(uint8_t type) {
    switch(type) {
      case 1: return 0;
      case 2: return 1;
      case 5: return 4;
      default: return -1;
    }
  }

  void Packet::initialize() {
    Packet::buffer[0] = 'S';
    memset(Packet::buffer + 1, 0, 10);
  }
  
  bool Packet::validate() {
    if(buffer[0] != 'S') return false;
    if(getTypeCode() == 0 || getTypeCode() > 5) return false;
    if(!type_has_pin(getTypeCode()) && getPin()) return false;
    if(buffer[7] & 0x7F) return false;
    if(buffer[8] || buffer[9]) return false;
    if(type_p_size(getTypeCode()) != -1 && type_p_size(getTypeCode()) != getPSize()) return false;
    
    return true;
  }
  
  uint32_t Packet::getAddress() {
    return (buffer[4] << 24) | (buffer[3] << 16) | (buffer[2] << 8) | buffer[1];
  }
  
  void Packet::setAddress(uint32_t v) {
    buffer[1] = v;
    buffer[2] = v >> 8;
    buffer[3] = v >> 16;
    buffer[4] = v >> 24;
  }
  
  void Packet::setTypeCode(TypeCode v) {
    buffer[5] = v;
    
    if(type_p_size(v) != -1) {
      buffer[10] = type_p_size(v);
    }
  }
  
  uint32_t Packet::getPayloadU32() {
    return (buffer[14] << 24) | (buffer[13] << 16) | (buffer[12] << 8) | buffer[11];
  }
  
  void Packet::setPayloadU32(uint32_t v) {
    payload()[0] = v;
    payload()[1] = v >> 8;
    payload()[2] = v >> 16;
    payload()[3] = v >> 24;
  }
}
