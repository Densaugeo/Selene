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
      bool type_has_pin(uint8_t type) {
        switch(type) {
          case 4: return true;
          case 5: return true;
          default: return false;
        }
      }
      
      /* type_size:
       *   Description:
       *     Given a type, check the size of its payload
       *   Parameters:
       *     type - Selene type code
       *   Returns:
       *     Size of payload, in bytes
       */
      uint8_t type_p_size(uint8_t type) {
        switch(type) {
          case 1: return 0;
          case 2: return 1;
          case 5: return 4;
          default: return -1;
        }
      }
    
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
      void initialize() {
        Packet::buffer[0] = 'S';
        memset(Packet::buffer + 1, 0, 10);
      }
      
      /* validate:
       *   Description:
       *     Check if this is a valid Selene packet
       *   Returns:
       *     true if valid
       */
      bool validate() {
        if(buffer[0] != 'S') return false;
        if(getTypeCode() == 0 || getTypeCode() > 5) return false;
        if(!type_has_pin(getTypeCode()) && getPin()) return false;
        if(buffer[7] & 0x7F) return false;
        if(buffer[8] || buffer[9]) return false;
        if(type_p_size(getTypeCode()) != -1 && type_p_size(getTypeCode()) != getPSize()) return false;
        
        return true;
      }
      
      /* size:
       *   Description:
       *     Get size of packet, in bytes
       *   Returns:
       *     Size of packet, in bytes
       */
      uint8_t size() { return 11 + getPSize(); }
      
      // Selene address
      uint32_t getAddress() {
        return (buffer[4] << 24) | (buffer[3] << 16) | (buffer[2] << 8) | buffer[1];
      }
      
      void setAddress(uint32_t v) {
        buffer[1] = v;
        buffer[2] = v >> 8;
        buffer[3] = v >> 16;
        buffer[4] = v >> 24;
      }
      
      // Selene type (by type code). Setting a type with a fixed payload size sets pSize byte
      TypeCode getTypeCode() { return (TypeCode) buffer[5]; }
      
      void setTypeCode(TypeCode v) {
        buffer[5] = v;
        
        if(type_p_size(v) != -1) {
          buffer[10] = type_p_size(v);
        }
      }
      
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
      uint32_t getPayloadU32() {
        return (buffer[14] << 24) | (buffer[13] << 16) | (buffer[12] << 8) | buffer[11];
      }
      
      void setPayloadU32(uint32_t v) {
        payload()[0] = v;
        payload()[1] = v >> 8;
        payload()[2] = v >> 16;
        payload()[3] = v >> 24;
      }
  };
}

#endif // ifndef
