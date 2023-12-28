#include "LSM6DSOXHandler.h"

LSM6DSOXHandler::LSM6DSOXHandler() {}

bool LSM6DSOXHandler::setup() {
  if (!sox.begin_I2C()) {
    return false;
  }
  // Additional configuration can be added here if needed
  return true;
}

String LSM6DSOXHandler::readSensorData() {
  sensors_event_t accel, gyro, temp;
  sox.getEvent(&accel, &gyro, &temp);

  String data;
  data += String(temp.temperature) + ",";
  data += String(accel.acceleration.x) + "," + String(accel.acceleration.y) + "," + String(accel.acceleration.z) + ",";
  data += String(gyro.gyro.x) + "," + String(gyro.gyro.y) + "," + String(gyro.gyro.z);

  return data;
}

void LSM6DSOXHandler::getEvent(sensors_event_t* accel, sensors_event_t* gyro, sensors_event_t* temp) {
  sox.getEvent(accel, gyro, temp);
}
