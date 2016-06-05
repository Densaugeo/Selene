/**
 * Represents a single Selene Device, with a Selene address and Pins
 */
#ifndef SELENE_DEVICE_H_INCLUDED
#define SELENE_DEVICE_H_INCLUDED

#include <stdint.h>
#include "Selene_Pin.hpp"
#include "Selene_PinPacket.hpp"
#include "Selene_Packet.hpp"

namespace Selene {
  class Device {
    private:
      // Cache a packet to use for pin updates
      PinPacket pinPacket;
      
      // If set, .sendPinUpdates() will send updates for all pins
      bool updateAllPins = false;
      
      /* send:
       *   Description:
       *     Sends Selene Packets over a network interface. User must provide a .send() function in constructor
       *   Parameters:
       *     payload - Pointer to data to send
       *     size - Size of payload, in bytes
       */
      void (*send)(uint8_t* payload, uint8_t size);
    
    public:
      // Selene address for this Device
      const uint32_t address;
      
      // Holds pointers to Selene Pins on this Device
      Pin** pins;
      
      // Number of Selene Pins in .pins
      const uint8_t len;
      
      // Info for Selene devinfo packets
      uint8_t* info;
      
      // Size of info, in bytes
      uint8_t infoSize;
      
      // Constructor just initializes stuff
      Device(uint32_t address, Pin** pins, uint8_t len, uint8_t* info, uint8_t infoSize, void (*send)(uint8_t*, uint8_t)):
      address(address), pins(pins), len(len), info(info), infoSize(infoSize), send(send), pinPacket(address) {}
      
      /* handlePacket:
       *   Description:
       *     Handles a Selene Packet. Sends replies to discovery packets. Pin updates are not sent until
       *     .sendPinUpdates() is called
       *   Parameters:
       *     buffer - Packet's raw data, held in its .buffer field
       */
      void handlePacket(uint8_t* buffer);
      
      /* replyToDiscovery:
       *   Description:
       *     Sends replies requested by discovery packets
       */
      void replyToDiscovery();
      
      /* pinRequest:
       *   Description:
       *     Handles a request to a pin, checking if pin exists and if requested state is allowed
       *   Parameters:
       *     pinNumber - Selene pin #
       *     state - Requested state
       */
      void pinRequest(uint8_t pinNumber, uint32_t state);
      
      /* sendPinUpdates:
       *   Description:
       *     Send updates for all Pins whose state has changed since the last time .sendPinUpdates() was called
       */
      void sendPinUpdates();
  };
}

#endif // ifndef
