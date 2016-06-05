/**
 * For reading and writing Selene packets
 */
#ifndef SELENE_PACKET_H_INCLUDED
#define SELENE_PACKET_H_INCLUDED

#include <stdint.h>

namespace Selene {
  class Packet {
    private:
      /* type_has_pin:
       *   Description:
       *     Given a type, check if it should have a pin field
       *   Parameters:
       *     type - Selene type code
       *   Returns:
       *     true if type should have a pin field
       */
      bool type_has_pin(uint8_t type);
      
      /* type_size:
       *   Description:
       *     Given a type, check the size of its payload
       *   Parameters:
       *     type - Selene type code
       *   Returns:
       *     Size of payload, in bytes
       */
      uint8_t type_p_size(uint8_t type);
    
    public:
      // Selene packet types
      enum TypeCode {
        DISCOVERY = 1,
        CONNECTION = 2,
        DEVINFO = 3,
        PININFO = 4,
        PIN = 5
      };
      
      // Holds the binary representation of the pin packet, which can be transmitted
      uint8_t* buffer;
      
      /* Packet:
       *   Description:
       *     Holdes a pointer to a binary packet, and provides methods for working with it
       *   Parameters:
       *     buffer - Pointer to memory area for binary packet
       *   Returns:
       *     A Packet with the given buffer. No other initialization is done
       */
      Packet(uint8_t* buffer) : buffer(buffer) {}
      
      /* initialize:
       *   Description:
       *     Initialize packet with prefix and zeroes for other fields. Will need to given
       *     address, type, pin (if applicable), and payload before use
       */
      void initialize();
      
      /* validate:
       *   Description:
       *     Check if this is a valid Selene packet
       *   Returns:
       *     true if valid
       */
      bool validate();
      
      /* size:
       *   Description:
       *     Get size of packet, in bytes
       *   Returns:
       *     Size of packet, in bytes
       */
      uint8_t size() { return 11 + getPSize(); }
      
      // Selene address
      uint32_t getAddress();
      void setAddress(uint32_t v);
      
      // Selene type (by type code). Setting a type with a fixed payload size sets pSize byte
      TypeCode getTypeCode() { return (TypeCode) buffer[5]; }
      void setTypeCode(TypeCode v);
      
      // Selene pin #
      uint8_t getPin() { return buffer[6]; }
      void setPin(uint8_t v) { buffer[6] = v; }
      
      // isRequest flag
      bool getIsRequest() { return buffer[7] & 0x80; }
      void setIsRequest(bool v) { buffer[7] = v << 7; }
      
      // Payload size, in bytes
      uint8_t getPSize() { return buffer[10]; }
      void setPSize(uint8_t v) { buffer[10] = v; }
      
      /* payload:
       *   Description:
       *     Provides a pointer to the payload section of the packet
       *   Returns:
       *     Pointer to payload
       */
      uint8_t* payload() { return buffer + 11; }
      
      // Payload as u32. Does not set pSize byte
      uint32_t getPayloadU32();
      void setPayloadU32(uint32_t v);
      
      /* writePayload:
       *   Description:
       *     Write a buffer into payload. Sets pSize byte
       *   Parameters:
       *     payload - Data to write into payload
       *     pSize - Size of payload, in bytes
       */
      void writePayload(uint8_t* payload, uint8_t pSize);
  };
}

#endif // ifndef
