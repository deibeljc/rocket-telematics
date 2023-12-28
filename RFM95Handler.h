#ifndef RFM95Handler_h
#define RFM95Handler_h

#include <Arduino.h>
#include <RH_RF95.h>

class RFM95Handler {
public:
    RFM95Handler();
    void setup();
    void sendMessage(const String& message);
    void receiveMessage();
    void blinkLED(byte pin, byte delay_ms, byte loops);

private:
    RH_RF95 rf95;
};

#endif
