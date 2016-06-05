#include "Selene_PinPacket.hpp"

namespace Selene {
  PinPacket::PinPacket(uint32_t address) {
    // Initializing buffer here saves dynamic memory
    buffer[0] = 'S';
    setAddress(address);
    buffer[5] = 5;
    buffer[6] = 0;
    buffer[7] = 0;
    buffer[8] = 0;
    buffer[9] = 0;
    buffer[10] = 4;
    buffer[11] = 0;
    buffer[12] = 0;
    buffer[13] = 0;
    buffer[14] = 0;
  }
  
  void PinPacket::setAddress(uint32_t v) {
    buffer[1] = v;
    buffer[2] = v >> 8;
    buffer[3] = v >> 16;
    buffer[4] = v >> 24;
  }
  
  void PinPacket::setPayloadU32(uint32_t v) {
    buffer[11] = v;
    buffer[12] = v >> 8;
    buffer[13] = v >> 16;
    buffer[14] = v >> 24;
  }
}
