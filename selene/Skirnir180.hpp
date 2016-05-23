/**
 * Arduino side of Skirnir. Responsible for handling serial packets
 */
#ifndef SKIRNIR180_H_INCLUDED
#define SKIRNIR180_H_INCLUDED

#include <HardwareSerial.h>
#include "Skirnir.hpp"

class Skirnir180 : public Skirnir {
  private:
    uint8_t receive_buffer[241];
  
  protected:
    /* fsmGlobals:
     *   Description:
     *     Same as Skirnir::fsmGlobals(), but also handles 180-byte packets
     */
    bool fsmGlobals(uint8_t payload[], uint8_t next, uint8_t input_buffer[]);
    
    /* fsmLocals:
     *   Description:
     *     Same as Skirnir::fsmLocals(), but also handles 180-byte packets
     */
    uint8_t fsmLocals(uint8_t payload[], uint8_t next, uint8_t input_buffer[]);
  
  public:
    /* Skirnir180:
     *   Description:
     *     Same as Skirnir, but can send payloads up to 180 bytes
     */
    Skirnir180(HardwareSerial* port);
    
    /* send180:
     *   Description:
     *     Sends a fixed-size 180-byte packet
     *   Parameters:
     *     payload - Bytes to send. The 180 bytes after this pointer are sent
     */
    void send180(uint8_t payload[]);
    
    /* send:
     *   Description:
     *     Same as Skirnir::send(), except this one can also send 180 byte packets
     *     Smaller payloads are sent in reulgar 45-byte packets, larger ones in 180-byte packets
     *   Parameters:
     *     payload - Bytes to send
     *     size - Size of payload. If size is more than 180, the first 180 bytes will be sent
     */
    void send(uint8_t payload[], uint32_t size);
    
    /* receive:
     *   Description:
     *     Same as Skirnir::receive(), but can receive both 45- and 180-byte packets
     */
    uint8_t receive(uint8_t payload[], uint8_t next);
    
    /* receive_until_packet:
     *   Description:
     *     Same as Skirnir::receive_until_packet(), but can receive both 45- and 180-byte packets
     */
    uint8_t receive_until_packet(uint8_t payload[]);
};

#endif // ifndef
