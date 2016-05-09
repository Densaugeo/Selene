/**
 * Arduino side of Skirnir. Responsible for handling serial packets
 */
#ifndef SKIRNIR_H_INCLUDED
#define SKIRNIR_H_INCLUDED

#include <HardwareSerial.h>

class Skirnir {
  private:
    enum FSMState {
      START,
      PING,
      PACKET_INTERMEDIATE,
      PACKET_END,
    };
    
    HardwareSerial* port;
    uint8_t receive_buffer[61];
    FSMState fsm_state;
    int fsm_repeats;
  
  public:
    /* Skirnir:
     *   Description:
     *     Skirnirs must be given a HardwareSerial reference when constructed
     *   Parameters:
     *     port - Reference to an Arduino-style HardwareSerial
     *   Returns:
     *     A Skirnir instance that is ready to use
     */
    Skirnir(HardwareSerial* port);
    
    /* heartbeat:
     *   Description:
     *     Initiates a heartbeat. Heartbeats must occur regularly; if the host PC does not detect a heartbeat for
     *     7 seconds, it will report a connection error
     */
    void heartbeat();
    
    /* send45:
     *   Description:
     *     Sends a fixed-size 45-byte packet. Lower overhead than .send()
     *   Parameters:
     *     payload - Bytes to send. The 45 bytes after this pointer are sent
     */
    void send45(uint8_t payload[]);
    
    /* send:
     *   Description:
     *     Sends a packet, with a payload of up to 45 bytes. Convenience wrapper for .send45()
     *   Parameters:
     *     payload - Bytes to send
     *     size - Size of payload. If size is more that 45, only 45 bytes will be sent
     */
    void send(uint8_t payload[], uint32_t size);
    
    /* receive:
     *   Description:
     *     Recieves a character. Responds to heartbeats and receives packets, if detected
     *   Parameters:
     *     payload - Array to store a packet, if received
     *     next - The character to receive
     *   Returns:
     *     true if a packet was detected
     */
    bool receive(uint8_t payload[], uint8_t next);
    
    /* receive_until_packet:
     *   Description:
     *     Recieves from port.read() until a packet is detected or input ends. Responds to heartbeats, if needed
     *   Parameters:
     *     payload - Array to store a packet, if received
     *   Returns:
     *     true if a packet was detected
     */
    bool receive_until_packet(uint8_t payload[]);
};

#endif // ifndef
