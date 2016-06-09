/**
 * Represents a single Selene Device, with a Selene address and Pins
 */
#ifndef SELENE_DEVICE_H_INCLUDED
#define SELENE_DEVICE_H_INCLUDED

#include "Selene_Pin.hpp"
#include "Selene_PinPacket.hpp"
#include "Selene_Packet.hpp"
#include <stdint.h>
#include <string.h>
#include <avr/pgmspace.h>

namespace Selene {
  class Device {
    private:
      // Cache a packet to use for pin updates
      PinPacket pinPacket;
      
      // Set to true when .sendPinUpdates() detects that a pin has changed
      bool needsSave = false;
      
      // Time last pin state change was detected, in ms
      uint32_t lastChange = 0;
      
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
      const uint8_t* info;
      
      // Size of info, in bytes
      const uint8_t infoSize;
      
      // True if .info points to a string in Arduino-style PROGMEM
      const bool infoProgmem;
      
      // If set, next .sendPinUpdates() will send updates for all pins
      bool sendAllPins = false;
      
      // Time to wait from last change before saving, in ms. Pin states are saved after all pins have remained
      // constant for this time. Setting this too low can burn out EEPROM!
      uint32_t saveDelay = 60000;
      
      // Constructor just initializes stuff
      Device(uint32_t address, Pin** pins, uint8_t len, const uint8_t* info, uint8_t infoSize, bool infoProgmem, void (*send)(uint8_t*, uint8_t)):
      address(address), pins(pins), len(len), info(info), infoSize(infoSize), infoProgmem(infoProgmem), send(send), pinPacket(address) {}
      
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
       *   Parameters:
       *     millis - Current time, as given by Arduino millis()
       */
      void sendPinUpdates(uint32_t millis);
      
      /* savePinStates:
       *   Description:
       *     If no pin changes have been detected for .saveDelay ms, call .saveState() on all Pins whose state
       *     has changed since the last save
       *   Parameters:
       *     millis - Current time, as given by Arduino millis()
       */
      void savePinStates(uint32_t millis);
  };
  
  void Device::handlePacket(uint8_t* buffer) {
    Packet packet = Packet(buffer);
    
    if(packet.validate() && (packet.getAddress() == address || packet.getAddress() == 0xFFFFFFFF)) {
      switch(packet.getTypeCode()) {
        case Packet::PIN:
          if(packet.getIsRequest()) {
            pinRequest(packet.getPin(), packet.getPayloadU32());
          }
          break;
        case Packet::DISCOVERY:
          replyToDiscovery();
      }
    }
  }
  
  void Device::replyToDiscovery() {
    uint8_t new_packet[155];
    Packet holder = Packet(new_packet);
    holder.initialize();
    holder.setAddress(address);
    
    // Send devinfo packet
    holder.setTypeCode(Packet::DEVINFO);
    holder.setPSize(infoSize);
    
    if(infoProgmem) {
      memcpy_P(holder.payload(), info, infoSize);
    } else {
      memcpy(holder.payload(), info, infoSize);
    }
    
    send(holder.buffer, holder.size());
    
    holder.setTypeCode(Packet::PININFO);
    
    for(uint8_t i = 0; i < len; ++i) {
      Pin* pin = pins[i];
      
      // And send a pininfo packet
      holder.setPin(pin -> selenePin);
      holder.setPSize(pin -> infoSize);
      
      if(pin -> infoProgmem) {
        memcpy_P(holder.payload(), pin -> info, pin -> infoSize);
      } else {
        memcpy(holder.payload(), pin -> info, pin -> infoSize);
      }
      
      send(holder.buffer, holder.size());
    }
    
    // And trigger pin updates
    sendAllPins = true;
  }
  
  void Device::pinRequest(uint8_t pinNumber, uint32_t state) {
    for(uint8_t i = 0; i < len; ++i) {
      Pin* pin = pins[i];
      
      if(pinNumber == pin -> selenePin) {
        pin -> setState(state);
      }
    }
  }
  
  void Device::sendPinUpdates(uint32_t millis) {
    for(uint8_t i = 0; i < len; ++i) {
      Pin* pin = pins[i];
      uint32_t state = pin -> getState();
      
      if(state != pin -> statePrevious || sendAllPins) {
        pinPacket.setPin(pin -> selenePin);
        pinPacket.setPayloadU32(state);
        send(pinPacket.buffer, pinPacket.size());
        
        pin -> statePrevious = state;
        needsSave = true;
        lastChange = millis;
      }
    }
    
    sendAllPins = false;
  }
  
  void Device::savePinStates(uint32_t millis) {
    if(needsSave && millis - lastChange > saveDelay) {
      for(uint8_t i = 0; i < len; ++i) {
        pins[i] -> saveState(address);
      }
      
      needsSave = false;
    }
  }
}

#endif // ifndef
