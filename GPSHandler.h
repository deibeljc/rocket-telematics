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
  uint8_t numberOfSattelites();
  uint8_t fixQuality();
  uint8_t fixQuality3d();
  nmea_float_t altitude();
  nmea_float_t initialAltitude();
  void setInitialAltitude();

private:
  Adafruit_GPS GPS;
  uint32_t timer;
  nmea_float_t initial_altitude;
  bool initial_altitude_set;
};

#endif
