#include "Selene_Device.hpp"

#include <string.h>
#include <avr/pgmspace.h>

namespace Selene {
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
