/**
 * Arduino side of Skirnir. Responsible for handling serial packets
 */
#ifndef SKIRNIR180_H_INCLUDED
#define SKIRNIR180_H_INCLUDED

#include "Skirnir.hpp"
#include <HardwareSerial.h>
#include "base64.hpp"

class Skirnir180 : public Skirnir {
  private:
    uint8_t receive_buffer[241];
  
  protected:
    /* fsmGlobals:
     *   Description:
     *     Same as Skirnir::fsmGlobals(), but also handles 180-byte packets
     */
    virtual bool fsmGlobals(uint8_t next) {
      switch(next) {
        case '&':
          fsmState = LPACKET_INTERMEDIATE;
          fsmRepeats = 0;
          return true;
        default:
          return Skirnir::fsmGlobals(next);
      }
    }
    
    /* fsmLocals:
     *   Description:
     *     Same as Skirnir::fsmLocals(), but also handles 180-byte packets
     */
    virtual uint8_t fsmLocals(uint8_t next) {
      switch(fsmState) {
        case LPACKET_INTERMEDIATE:
          getReceiveBuffer()[fsmRepeats] = next;
          if(++fsmRepeats >= 240) fsmState = LPACKET_END;
          return 0;
        case LPACKET_END:
          fsmState = START;
          if(next == '\n') {
            decode_base64(getReceiveBuffer(), getReceiveBuffer());
            return 180;
          }
          return 0;
        default:
          return Skirnir::fsmLocals(next);
      }
    }
  
  public:
    // Publicly accessible pointer to the receive buffer
    virtual uint8_t* getReceiveBuffer() { return receive_buffer; }
    
    /* Skirnir180:
     *   Description:
     *     Same as Skirnir, but can receive 180-byte packets (at the cost of using more RAM)
     */
    Skirnir180(HardwareSerial* port): Skirnir(port) {
      receiveBuffer = getReceiveBuffer();
      receive_buffer[240] = '\0';
    }
};

#endif // ifndef
