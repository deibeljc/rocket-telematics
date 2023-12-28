#include "GPSHandler.h"

// The name of the hardware serial port:
#define GPSSerial Serial1

GPSHandler::GPSHandler() : GPS(&GPSSerial), timer(0), initial_altitude_set(false) {}

String previousGPS = ",";

void GPSHandler::setup() {
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);  // 5 Hz update rate
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(5000);
}

void GPSHandler::update() {
  char c = GPS.read();
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) {
      return;  // We failed to parse a sentence, so wait for another
    }
  }
}

bool GPSHandler::hasFix() {
  return GPS.fix;
}

uint8_t GPSHandler::numberOfSattelites() {
  // return 0;
  return GPS.satellites;
}

uint8_t GPSHandler::fixQuality() {
  // return 0;
  return GPS.fixquality;
}

uint8_t GPSHandler::fixQuality3d() {
  return GPS.fixquality_3d;
}

nmea_float_t GPSHandler::altitude() {
  return GPS.altitude;
}

String GPSHandler::readGPS() {
  if (GPS.fix) {
    String data = String(GPS.latitude_fixed / 10000000.0, 10) + "," + String(GPS.longitude_fixed / 10000000.0, 10);
    previousGPS = data;

    return data;
  } else {
    return previousGPS;
  }

  return previousGPS;
}

void GPSHandler::printGPSInfo() {
  if (millis() - timer > 2000) {
    timer = millis();  // Reset the timer

    // Check if we have a fix
    if (GPS.fix) {
      // Latitude conversion
      float latitudeDegrees = (int)(GPS.latitude / 100);
      float latitudeMinutes = GPS.latitude - (latitudeDegrees * 100);
      float decimalLat = latitudeDegrees + (latitudeMinutes / 60);
      if (GPS.lat == 'S') decimalLat *= -1;  // Handle South latitudes

      // Longitude conversion
      float longitudeDegrees = (int)(GPS.longitude / 100);
      float longitudeMinutes = GPS.longitude - (longitudeDegrees * 100);
      float decimalLon = longitudeDegrees + (longitudeMinutes / 60);
      if (GPS.lon == 'W') decimalLon *= -1;  // Handle West longitudes

      Serial.print("Location (Decimal Degrees): ");
      Serial.print(decimalLat, 6);  // 6 decimal places for accuracy
      Serial.print(", ");
      Serial.println(decimalLon, 6);  // 6 decimal places for accuracy
    } else {
      Serial.println("GPS not fixed yet");
    }
  }
}
