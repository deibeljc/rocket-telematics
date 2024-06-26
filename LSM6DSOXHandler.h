#ifndef LSM6DSOX_HANDLER_H
#define LSM6DSOX_HANDLER_H

#include <Adafruit_LSM6DSOX.h>

class LSM6DSOXHandler {
public:
  LSM6DSOXHandler();
  bool setup();
  String readSensorData();
  void getEvent(sensors_event_t* accel, sensors_event_t* gyro, sensors_event_t* temp);

private:
  Adafruit_LSM6DSOX sox;
};

#endif
