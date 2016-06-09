/**
 * A pin-only Selene packet, to be used for frequent pin updates
 */
#ifndef SELENE_PINPACKET_H_INCLUDED
#define SELENE_PINPACKET_H_INCLUDED

#include <stdint.h>
#include <string.h>

namespace Selene {
  class PinPacket {
    public:
      // Holds the binary representation of the pin packet, which can be transmitted
      uint8_t buffer[15];
      
      /* PinPacket:
       *   Description:
       *     Represents a Selene packet of type pin. Cannot be used for pin/r, only for update packets
       *   Parameters:
       *     address - Selene address
       */
      PinPacket(uint32_t address) {
        // Initializing buffer here saves dynamic memory
        buffer[0] = 'S';
        setAddress(address);
        buffer[5] = 5;
        memset(buffer + 6, 0, 9);
        buffer[10] = 4;
      }
      
      /* size:
       *   Description:
       *     Get size of packet
       *   Returns:
       *     Size of packet in bytes (always 15)
       */
      uint8_t size() { return 15; }
      
      /* setAddress:
       *   Description:
       *     Sets Selene address
       */
      void setAddress(uint32_t v) {
        buffer[1] = v;
        buffer[2] = v >> 8;
        buffer[3] = v >> 16;
        buffer[4] = v >> 24;
      }
      
      /* setPin:
       *   Description:
       *     Sets Selene pin #
       */
      void setPin(uint8_t v) { buffer[6] = v; }
      
      /* setPayloadU32:
       *   Description:
       *     Sets payload using a u32
       */
      void setPayloadU32(uint32_t v) {
        buffer[11] = v;
        buffer[12] = v >> 8;
        buffer[13] = v >> 16;
        buffer[14] = v >> 24;
      }
  };
}

#endif // ifndef
