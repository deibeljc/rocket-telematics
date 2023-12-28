#ifndef GPS_HANDLER_H
#define GPS_HANDLER_H

#include <Adafruit_GPS.h>

class GPSHandler {
public:
    GPSHandler();
    void setup();
    void update();
    void printGPSInfo();
    bool hasFix();
    String readGPS();

private:
    Adafruit_GPS GPS;
    uint32_t timer;
};

#endif
