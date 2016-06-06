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
     *     Same as Skirnir, but can receive 180-byte packets (at the cost of using more RAM)
     */
    Skirnir180(HardwareSerial* port);
    
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
