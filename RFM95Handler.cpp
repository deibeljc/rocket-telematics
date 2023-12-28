#include "RFM95Handler.h"
#include "RHSoftwareSPI.h"

#define RF95_FREQ 915.0
#define RFM95_CS A4
#define RFM95_RST A2
#define RFM95_INT A3

RHSoftwareSPI spi;

RFM95Handler::RFM95Handler() : rf95(RFM95_CS, RFM95_INT, spi) {}

void RFM95Handler::setup() {
    Serial.println("Feather RFM95 Setup");

    if (!rf95.init()) {
        Serial.println("RFM95 radio init failed");
        while (1);
    }

    if (!rf95.setFrequency(RF95_FREQ)) {
        Serial.println("setFrequency failed");
    }

    rf95.setTxPower(23, false);
}

void RFM95Handler::sendMessage(const String& message) {
    rf95.send((uint8_t *)message.c_str(), message.length());
    rf95.waitPacketSent();
}

void RFM95Handler::receiveMessage() {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.waitAvailableTimeout(500)) {
        if (rf95.recv(buf, &len)) {
            Serial.print("Received: ");
            Serial.println((char*)buf);
            blinkLED(LED_BUILTIN, 50, 3);
        } else {
            Serial.println("Receive failed");
        }
    } else {
        Serial.println("No reply, is another RFM95 listening?");
    }
}

void RFM95Handler::blinkLED(byte pin, byte delay_ms, byte loops) {
    for(byte i = 0; i < loops; i++) {
        digitalWrite(pin, HIGH);
        delay(delay_ms);
        digitalWrite(pin, LOW);
        delay(delay_ms);
    }
}
