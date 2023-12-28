#ifndef BMP_HANDLER_H
#define BMP_HANDLER_H

#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"

#define BMP_CS 5
#define SEALEVELPRESSURE_HPA (1009.5)

class BMPHandler {
public:
  BMPHandler();
  bool setup();
  float readAltitude();
  void setAltitudeOffset(float offset);

private:
  Adafruit_BMP3XX bmp;
  float altitude_offset;
};

#endif
