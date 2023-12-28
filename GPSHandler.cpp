#include "GPSHandler.h"

// The name of the hardware serial port:
#define GPSSerial Serial1

GPSHandler::GPSHandler() : GPS(&GPSSerial), timer(0) {}

String previousGPS = ",";

void GPSHandler::setup() {
    GPS.begin(9600);
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);  // 1 Hz update rate
    GPS.sendCommand(PGCMD_ANTENNA);

    delay(1000);  // Wait a bit for the GPS to get a fix

    GPSSerial.println(PMTK_Q_RELEASE);  // Ask for firmware version
}

void GPSHandler::update() {
    char c = GPS.read();
    if (GPS.newNMEAreceived()) {
        if (!GPS.parse(GPS.lastNMEA())) {
            return; // We failed to parse a sentence, so wait for another
        }
    }
}

bool GPSHandler::hasFix() {
  return GPS.fix;
}

String GPSHandler::readGPS() {
  if (millis() - timer > 2000) {
        timer = millis(); // Reset the timer

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

            String data = String(decimalLat, 8) + "," + String(decimalLon, 8);
            previousGPS = data;

            return data;
        } else {
            return previousGPS;
        }
    }

    return previousGPS;
}

void GPSHandler::printGPSInfo() {
    if (millis() - timer > 2000) {
        timer = millis(); // Reset the timer

        // Check if we have a fix
        if (GPS.fix) {
            Serial.print("Time: ");
            // ... existing code to print time ...

            Serial.print("Date: ");
            // ... existing code to print date ...

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
        }
        else {
            Serial.println("GPS not fixed yet");
        }
    }
}
