#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include "util.h" // Include the utility header file

enum RocketState {
  LANDED,
  LAUNCHING,
  APEX,
  DESCENDING,
  CALIBRATION
};

RocketState currentState = LANDED;
RocketState previousState = LANDED;

// Our modules
#include "flash_config.h"
#include "FileHandler.h"
#include "BMPHandler.h"
#include "LSM6DSOXHandler.h"
#include "GPSHandler.h"
#include "RFM95Handler.h"
#include "LaunchDetector.h"

#define ALTITUDE_DATA_FILE "/alt.txt"
#define RED_LIGHT_PIN 13  // Define the pin for the red light

FatFileSystem fatfs;
BMPHandler bmpHandler;
LSM6DSOXHandler soxHandler;
GPSHandler gpsHandler;
RFM95Handler rfm95Handler;
LaunchDetector launchDetector(bmpHandler, soxHandler);

Adafruit_SPIFlash flash(&flashTransport);
FileHandler fileHandler(fatfs, ALTITUDE_DATA_FILE);

String inputString = "";
bool stringComplete = false;

unsigned long lastTime = 0;     // Timer for calculating loops per second
bool debugLogging = false;      // Flag to control debug logging
unsigned long loopCounter = 0;  // Counter for number of loops

bool isRecording = false;
float maxAltitude = 0.0;
bool bufferFull = false;  // Flag to check if buffer is full

float altitudeBuffer[ALTITUDE_BUFFER_SIZE];
float runningSum = 0.0;
float runningSumOfSquares = 0.0;
float groundLevelAltitude = 0.0;
const int CALIBRATION_READINGS = 100;

int altitudeBufferIndex = 0;
float altitudeWindow[SMOOTHING_WINDOW_SIZE];
int altitudeWindowIndex = 0;
bool altitudeWindowFull = false;

unsigned long setupBlinkTimer = 0;
unsigned long loopBlinkTimer = 0;

void println(String output) {
  if (Serial) {
    Serial.println(output);
  }
}

String rocketStateToString(RocketState state) {
  switch (state) {
    case LANDED:
      return "LANDED";
    case LAUNCHING:
      return "LAUNCHING";
    case APEX:
      return "APEX";
    case DESCENDING:
      return "DESCENDING";
    case CALIBRATION:
      return "CALIBRATION";
    default:
      return "UNKNOWN";
  }
}

void serialEvent() {
  static String serialBuffer = "";
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    serialBuffer += inChar;
    if (inChar == '\n') {
      inputString = serialBuffer;
      stringComplete = true;
      serialBuffer = "";
    }
  }
}

void print(String output) {
  if (Serial) {
    Serial.print(output);
  }
}

void blinkRedLight(unsigned long interval, unsigned long &timer) {
  if (millis() - timer >= interval) {
    digitalWrite(RED_LIGHT_PIN, !digitalRead(RED_LIGHT_PIN));
    timer = millis();
  }
}

void initializeHandlers() {
  if (!bmpHandler.setup()) {
    println("Could not find a valid BMP3 sensor, check wiring!");
    while (1);
  }

  if (!soxHandler.setup()) {
    println("Failed to find LSM6DSOX chip");
    while (1) {
      delay(10);
    }
  }

  if (!flash.begin()) {
    println("Error initializing flash!");
    while (1);
  }

  if (!fatfs.begin(&flash)) {
    println("Error initializing filesystem!");
    while (1);
  }

  rfm95Handler.setup();
  gpsHandler.setup();

  rfm95Handler.sendMessage("Trying to get GPS Fix " + String(gpsHandler.numberOfSattelites()));
  println("Trying to get GPS Fix " + String(gpsHandler.numberOfSattelites()));
  while (!gpsHandler.hasFix()) {  // Wait for a GPS fix
    if (debugLogging) println("Waiting for GPS fix...");
    checkAndHandleSerialEvent();
    delay(100);
    gpsHandler.update();  // Update GPS data
  }
  println("GPS Fix: " + String(gpsHandler.numberOfSattelites()));
  
  // Calibration phase to determine ground level altitude
  float altitudeReadings[CALIBRATION_READINGS];
  for (int i = 0; i < CALIBRATION_READINGS; i++) {
    altitudeReadings[i] = bmpHandler.readAltitude();
    delay(50);  // Small delay between readings
  }
  int validReadings = CALIBRATION_READINGS;
  removeOutliers(altitudeReadings, validReadings);

  float altitudeSum = 0.0;
  for (int i = 0; i < validReadings; i++) {
    altitudeSum += altitudeReadings[i];
  }
  groundLevelAltitude = altitudeSum / validReadings;
  bmpHandler.setAltitudeOffset(groundLevelAltitude);  // Set altitude offset using average altitude
  println("Calibrated Ground Level Altitude: " + String(groundLevelAltitude));
  rfm95Handler.sendMessage("Calibrated Ground Level Altitude: " + String(groundLevelAltitude));

  // Lets write our calibration log here so we can use it as a relative point
  // for gyro and accel readings
  String calibrationTelematics = "";
  String sensorData = soxHandler.readSensorData();
  calibrationTelematics += String(millis()) + ",";
  calibrationTelematics += String(sensorData) + ",";
  calibrationTelematics += String(0) + ",";
  calibrationTelematics += String(gpsHandler.readGPS()) + ",";
  calibrationTelematics += rocketStateToString(CALIBRATION);
  fileHandler.addToBatch(calibrationTelematics);
  fileHandler.writeBatchToFile();
  
  fileHandler.setupFile();
  inputString.reserve(200);
}

void resetSystem() {
  // Clear the data buffer
  fileHandler.clearBatch();
  
  // Delete the file
  fileHandler.deleteFile();
  
  // Reset global variables
  currentState = LANDED;
  previousState = LANDED;
  inputString = "";
  stringComplete = false;
  lastTime = 0;
  loopCounter = 0;
  isRecording = false;
  maxAltitude = 0.0;
  bufferFull = false;
  altitudeBufferIndex = 0;
  altitudeWindowIndex = 0;
  altitudeWindowFull = false;
  setupBlinkTimer = 0;
  loopBlinkTimer = 0;
  runningSum = 0.0;
  runningSumOfSquares = 0.0;
  for (int i = 0; i < ALTITUDE_BUFFER_SIZE; i++) {
    altitudeBuffer[i] = 0.0;
  }
  for (int i = 0; i < SMOOTHING_WINDOW_SIZE; i++) {
    altitudeWindow[i] = 0.0;
  }
  runningSum = 0.0;
  runningSumOfSquares = 0.0;
  println("System reset");
}

void checkAndHandleSerialEvent() {
  serialEvent();
  if (stringComplete) {
    if (inputString == "r\n") {
      fileHandler.sendFileDataOverSerial();
    } else if (inputString == "clear\n") {
      // Reset altitude buffer
      fileHandler.clearBatch();
      println("Cleared batch");
    } else if (inputString == "delete\n") {
      fileHandler.deleteFile();
    } else if (inputString == "reset\n") {
      resetSystem();
    } else if (inputString == "debug on\n") {
      debugLogging = true;
      println("Debug logging enabled");
    } else if (inputString == "debug off\n") {
      debugLogging = false;
      println("Debug logging disabled");
    }
    inputString = "";
    stringComplete = false;
  }
}

void updateTelemetry(float altitude, String &telematics, String &packetToSend) {
  String sensorData = soxHandler.readSensorData();
  telematics += String(millis()) + ",";
  telematics += String(sensorData) + ",";
  telematics += String(altitude) + ",";
  telematics += String(gpsHandler.readGPS()) + ",";
  telematics += rocketStateToString(currentState);
  if (currentState != LANDED) {
    fileHandler.addToBatch(telematics);
  }

  // Check if one second has passed
  if (millis() - lastTime >= 5000) {
    String sensorData = soxHandler.readSensorData();
    packetToSend += String(millis()) + ",";
    packetToSend += String(sensorData) + ",";
    packetToSend += String(altitude) + ",";
    packetToSend += String(gpsHandler.readGPS());

    rfm95Handler.sendMessage(packetToSend);

    // Reset counter and timer
    loopCounter = 0;
    lastTime = millis();
  }
}

void addAltitudeToBuffer(float altitude) {
  altitudeBuffer[altitudeBufferIndex] = altitude;
  altitudeBufferIndex = (altitudeBufferIndex + 1) % ALTITUDE_BUFFER_SIZE;

  // Check if buffer has wrapped around at least once
  if (altitudeBufferIndex == 0) {
    bufferFull = true;
  }
}

void setup() {
  Serial.begin(115200);
  println("Adafruit BMP388 / BMP390 test");

  pinMode(RED_LIGHT_PIN, OUTPUT);
  digitalWrite(RED_LIGHT_PIN, LOW);

  initializeHandlers();

  lastTime = millis();
}

void loop() {
  loopCounter++;  // Increment loop counter

  float rawAltitude = -1;
  float smoothedAltitude = -1;
  float stdDev = -1;
  float rateOfChange = 0.0;
  static float previousAltitude = 0.0;

  rawAltitude = bmpHandler.readAltitude();
  smoothedAltitude = calculateMovingAverage(rawAltitude);
  addAltitudeToBuffer(smoothedAltitude);
  stdDev = computeStandardDeviation();
  rateOfChange = smoothedAltitude - previousAltitude;
  previousAltitude = smoothedAltitude;

  // Send data to Serial Plotter and debug log
  if (debugLogging) {
    Serial.print("Altitude:");
    Serial.print(smoothedAltitude);
    Serial.print(",");
    Serial.print("RateOfChange:");
    Serial.print(rateOfChange);
    Serial.print(",");
    Serial.print("StdDev:");
    Serial.println(stdDev);
  }
  // State machine logic
  if (currentState == LANDED) {
    if (bufferFull && stdDev > 0.1 && rateOfChange >= 0.02) {
      currentState = LAUNCHING;
      println("State changed to LAUNCHING");
    }
  } else if (currentState == LAUNCHING) {
    if (smoothedAltitude > maxAltitude) {
      maxAltitude = smoothedAltitude;
    }
    // Detect the apex by checking if the altitude is decreasing
    if (rateOfChange < -0.01) {  // The threshold can be adjusted based on your requirements
      currentState = DESCENDING;
      println("State changed to DESCENDING");
    }
  } else if (currentState == DESCENDING) {
    if (stdDev < 0.15 && rateOfChange < 0.01 && rateOfChange > -0.01) {
      currentState = LANDED;
      println("State changed to LANDED");
      String landedTelematics = "";
      String sensorData = soxHandler.readSensorData();
      landedTelematics += String(millis()) + ",";
      landedTelematics += String(sensorData) + ",";
      landedTelematics += String(rawAltitude) + ",";
      landedTelematics += String(gpsHandler.readGPS()) + ",";
      landedTelematics += rocketStateToString(currentState);
      fileHandler.addToBatch(landedTelematics);
      // Ensure we flush the data once we have landed.
      fileHandler.writeBatchToFile();
    }
  }
  previousState = currentState;

  String telematics = "";
  String packetToSend = "";

  updateTelemetry(rawAltitude, telematics, packetToSend);
  checkAndHandleSerialEvent();
  gpsHandler.update();

  blinkRedLight(1000, loopBlinkTimer);  // Blink the red light every second in the main loop
}
