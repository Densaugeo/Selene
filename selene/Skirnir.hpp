/**
 * Arduino side of Skirnir. Responsible for handling serial packets
 */
#ifndef SKIRNIR_H_INCLUDED
#define SKIRNIR_H_INCLUDED

#include <HardwareSerial.h>
#include "base64.hpp"

class Skirnir {
  private:
    // Where raw packets are stored as they come in, and where finished packets are put when they are found
    uint8_t receiveBufferPrivate[61];
  
  protected:
    enum FSMState {
      START,
      PING,
      PACKET_INTERMEDIATE,
      PACKET_END,
      LPACKET_INTERMEDIATE,
      LPACKET_END
    };
    
    HardwareSerial* port;
    FSMState fsmState = START;
    int fsmRepeats;
    
    /* fsmGlobals:
     *   Description:
     *     First stage of FSM used by receive()
     *   Parameters:
     *     next - The character to receive
     *   Returns:
     *     true if next character triggered a global transition
     */
    virtual bool fsmGlobals(uint8_t next) {
      switch(next) {
        case '-':
          fsmState = PING;
          return true;
        case '#':
          fsmState = PACKET_INTERMEDIATE;
          fsmRepeats = 0;
          getReceiveBuffer()[60] = '\0';
          return true;
        default:
          return false;
      }
    }
    
    /* fsmLocals:
     *   Description:
     *     Second stage of FSM used by receive()
     *   Parameters:
     *     next - The character to receive
     *   Returns:
     *     if a packet was completed, size in bytes (always 45). Otherwise, zero
     */
    virtual uint8_t fsmLocals(uint8_t next) {
      switch(fsmState) {
        case PING:
          if(next == '\n') port -> write(">\n");
          fsmState = START;
          return 0;
        case PACKET_INTERMEDIATE:
          getReceiveBuffer()[fsmRepeats] = next;
          if(++fsmRepeats >= 60) fsmState = PACKET_END;
          return 0;
        case PACKET_END:
          fsmState = START;
          if(next == '\n') {
            decode_base64(getReceiveBuffer(), getReceiveBuffer());
            return 45;
          }
          return 0;
        default:
          return 0;
      }
    }
    
    // Get pointer to correct receive buffer
    virtual uint8_t* getReceiveBuffer() { return receiveBufferPrivate; }
  
  public:
    // Publicly accessible pointer to the receive buffer
    uint8_t* receiveBuffer;
    
    /* Skirnir:
     *   Description:
     *     Sends and receives 45-byte packets over an Arduino-compatible HardwareSerial port. Can send 180-byte packets,
     *     but can only receive 45-byte packets
     *   Parameters:
     *     port - Reference to an Arduino-style HardwareSerial
     *   Returns:
     *     A Skirnir instance that is ready to use
     */
    Skirnir(HardwareSerial* port): port(port) {
      receiveBuffer = getReceiveBuffer();
    }
    
    /* heartbeat:
     *   Description:
     *     Initiates a heartbeat. Heartbeats must occur regularly; if the host PC does not detect a heartbeat for
     *     7 seconds, it will report a connection error
     */
    void heartbeat() {
      port -> write("<\n");
    }
    
    /* send:
     *   Description:
     *     Sends a packet, with a payload of up to 180 bytes
     *     Smaller payloads are sent in 45-byte packets, larger ones in 180-byte packets
     *   Parameters:
     *     payload - Bytes to send
     *     size - Size of payload. If size is more than 180, the first 180 bytes will be sent
     */
    void send(uint8_t payload[], uint32_t size) {
      uint8_t payload_size = size < 180 ? size : 180;
      uint8_t packet_size = size <= 45 ? 60 : 240;
      
      uint8_t send_buffer[5];
      send_buffer[4] = '\0';
      
      // Skirnir packet prefix
      port -> write(size <= 45 ? '#' : '&');
      
      // Base64-encode payload in blocks of 3
      for(uint8_t i = 0; i < payload_size - 2; i += 3) {
        encode_base64(payload + i, 3, send_buffer);
        port -> write((char*) send_buffer);
      }
      
      // Check for a partial block at end of payload
      if(payload_size % 3 != 0) {
        send_buffer[1] = payload[payload_size/3*3];
        send_buffer[2] = payload_size % 3 == 2 ? payload[payload_size/3*3 + 1] : 0;
        send_buffer[3] = 0;
        
        encode_base64(send_buffer + 1, 3, send_buffer);
        port -> write((char*) send_buffer);
      }
      
      // Fill remaineder of packet with base64 zeros ('A' characters)
      for(uint8_t i = (payload_size + 2)/3*4; i < packet_size; ++i) {
        port -> write('A');
      }
      
      port -> write('\n');
    }
    
    /* receive:
     *   Description:
     *     Recieves a character. Responds to heartbeats and receives packets, if detected. If a packet is found,
     *     it is parsed and left in .receiveBuffer. Received packets are overwritten by .receive() and
     *     .receiveUntilPacket()
     *   Parameters:
     *     next - The character to receive
     *   Returns:
     *     if a packet was detected, size in bytes (always 45). Otherwise, zero
     */
    uint8_t receive(uint8_t next) {
      if(fsmGlobals(next)) {
        return 0;
      } else {
        return fsmLocals(next);
      }
    }
    
    /* receiveUntilPacket:
     *   Description:
     *     Recieves from port.read() until a packet is detected or input ends. Responds to heartbeats, if detected.
     *     If a packet is found, it is parsed and left in .receiveBuffer. Received packets are overwritten by
     *     .receive() and .receiveUntilPacket()
     *   Returns:
     *     if a packet was detected, size in bytes (always 45). Otherwise, zero
     */
    uint8_t receiveUntilPacket() {
      uint8_t result = 0;
      
      while(port -> available()) {
        result = receive(port -> read());
        
        if(result) {
          return result;
        }
      }
      
      return 0;
    }
};

#endif // ifndef
