#include "LaunchDetector.h"

LaunchDetector::LaunchDetector(BMPHandler& bmpHandler, LSM6DSOXHandler& soxHandler)
    : bmp(bmpHandler), sox(soxHandler), launched(false), landed(false), lastAltitude(0.0f), lastUpdateTime(0) {}

void LaunchDetector::update() {
    Serial.println("LaunchDetector::update() called");
    if (launched) {
        Serial.println("Already launched, returning");
        return; // No need to update if already launched
    }

    float currentAltitude = bmp.readAltitude();
    Serial.println("Current altitude: " + String(currentAltitude));
    sensors_event_t accel;
    sox.getEvent(&accel, nullptr, nullptr);
    Serial.println("Acceleration event retrieved");

    if (launched && !landed) {
        Serial.println("Launched and not landed");
        // Check for landing
        float currentAltitude = bmp.readAltitude();
        Serial.println("Current altitude: " + String(currentAltitude));
        if ((lastAltitude - currentAltitude) >= LANDING_ALTITUDE_THRESHOLD) {
            landed = true;
            Serial.println("Landing detected!");
        }
    } else {
        Serial.println("Not launched or landed");
        unsigned long currentTime = millis();
        Serial.println("Current time: " + String(currentTime));
        if (currentTime - lastUpdateTime >= LAUNCH_TIME_THRESHOLD) {
            Serial.println("Launch time threshold exceeded");
            // X is our down direction since the board is oriented in the payload oddly.
            // positive 1g is when the rocket is pointing straight up.
            if (launched && !landed) {
                Serial.println("Launched and not landed");
                // Check for landing
                float currentAltitude = bmp.readAltitude();
                Serial.println("Current altitude: " + String(currentAltitude));
                if ((lastAltitude - currentAltitude) >= LANDING_ALTITUDE_THRESHOLD) {
                    landed = true;
                    Serial.println("Landing detected!");
                }
            } else {
                Serial.println("Not launched or landed");
                unsigned long currentTime = millis();
                Serial.println("Current time: " + String(currentTime));
                if (currentTime - lastUpdateTime >= LAUNCH_TIME_THRESHOLD) {
                    Serial.println("Launch time threshold exceeded");
                    // X is our down direction since the board is oriented in the payload oddly.
                    // positive 1g is when the rocket is pointing straight up.
                    unsigned long currentTime = millis();
                    Serial.println("Current time: " + String(currentTime));
                    if (currentTime - lastUpdateTime >= LAUNCH_TIME_THRESHOLD) {
                        Serial.println("Launch time threshold exceeded");
                        // X is our down direction since the board is oriented in the payload oddly.
                        // positive 1g is when the rocket is pointing straight up.
                        if ((currentAltitude - lastAltitude) > LAUNCH_ALTITUDE_THRESHOLD
                            && accel.acceleration.x > LAUNCH_ACCELERATION_THRESHOLD) {
                            launched = true;
                            Serial.println("Launch detected!");
                        }
                        lastUpdateTime = currentTime;
                        Serial.println("Last update time updated: " + String(lastUpdateTime));
                        lastAltitude = currentAltitude;
                        Serial.println("Last altitude updated: " + String(lastAltitude));
                    }
                }
            }
        }
    }
}

bool LaunchDetector::isLaunched() {
    Serial.println("LaunchDetector::isLaunched() called, returning: " + String(launched));
    return launched;
}

bool LaunchDetector::hasLanded() {
    Serial.println("LaunchDetector::hasLanded() called, returning: " + String(landed));
    return landed;
}