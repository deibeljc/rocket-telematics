#ifndef LAUNCH_DETECTOR_H
#define LAUNCH_DETECTOR_H

#include "BMPHandler.h"
#include "LSM6DSOXHandler.h"

class LaunchDetector {
public:
  LaunchDetector(BMPHandler& bmpHandler, LSM6DSOXHandler& soxHandler);
  void update();
  bool isLaunched();
  bool hasLanded();

private:
  BMPHandler& bmp;
  LSM6DSOXHandler& sox;
  bool launched;
  bool landed;
  float lastAltitude;
  unsigned long lastUpdateTime;
  static constexpr float LAUNCH_ACCELERATION_THRESHOLD = 29.4f;  // Approx 3g in m/s^2
  static constexpr float LAUNCH_ALTITUDE_THRESHOLD = 3.0f;      // 3 meters altitude increase
  static constexpr float LANDING_ALTITUDE_THRESHOLD = 3.0f;      // 3 meters altitude decrease
  static constexpr unsigned long LAUNCH_TIME_THRESHOLD = 1000;
};

#endif  // LAUNCH_DETECTOR_H
