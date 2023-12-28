#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>

// Our modules
#include "flash_config.h"
#include "FileHandler.h"
#include "BMPHandler.h"
#include "LSM6DSOXHandler.h"
#include "GPSHandler.h"
#include "RFM95Handler.h"

#define ALTITUDE_DATA_FILE "/altitude2.txt"
#define ALTITUDE_BUFFER_SIZE 50

FatFileSystem fatfs;
BMPHandler bmpHandler;
LSM6DSOXHandler soxHandler;
GPSHandler gpsHandler;
RFM95Handler rfm95Handler;

Adafruit_SPIFlash flash(&flashTransport);
FileHandler fileHandler(fatfs, ALTITUDE_DATA_FILE);

String inputString = "";
bool stringComplete = false;

unsigned long lastTime = 0; // Timer for calculating loops per second
unsigned long loopCounter = 0; // Counter for number of loops

void println(String output) {
  if (Serial) {
    Serial.println(output);
  }
}

void print(String output) {
  if (Serial) {
    Serial.print(output);
  }
}

void setup() {
    Serial.begin(115200);
    println("Adafruit BMP388 / BMP390 test");

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

    gpsHandler.setup();
    fileHandler.setupFile();
    rfm95Handler.setup();
    inputString.reserve(200);
    lastTime = millis();
}

bool isRecording = false;
bool justStartedRecording = false;
bool justStoppedRecording = false;

void loop() {
    String telematics = "";
    String packetToSend = "";
    loopCounter++; // Increment loop counter

    float altitude = bmpHandler.readAltitude();
    addAltitudeToBuffer(altitude);


    // Compute the current standard deviation
    float currentStdDev = computeStandardDeviation();
    float stdDev = 0.11;

    // Serial.println(String(altitude, 3) + " " + String(currentStdDev, 5));
    // Check if we need to start or stop recording
    if (currentStdDev > stdDev) {
        println("Started recording data: " + String(currentStdDev));
        isRecording = true;
    } else if (currentStdDev <= stdDev) {
        isRecording = false;
    }

    if (isRecording) {
        String sensorData = soxHandler.readSensorData();
        telematics += String(millis()) + ",";
        telematics += String(sensorData) + ",";
        telematics += String(altitude) + ",";
        telematics += gpsHandler.readGPS();
        fileHandler.addToBatch(telematics);
    } else {
      fileHandler.writeBatchToFile();
    }

    serialEvent();

    if (stringComplete) {
        if (inputString == "r\n") {
            fileHandler.sendFileDataOverSerial();
        }
        else if (inputString == "delete\n") {
            fileHandler.deleteFile();
        }
        inputString = "";
        stringComplete = false;
    }

    // Check if one second has passed
    if (millis() - lastTime >= 1000) {
        // Calculate and print loops per second
        print("Loops per second: ");
        println(String(loopCounter));

        println("Has GPS Fix: " + String(gpsHandler.hasFix()));

        String sensorData = soxHandler.readSensorData();
        packetToSend += String(millis()) + ",";
        packetToSend += String(sensorData) + ",";
        packetToSend += String(altitude) + ",";
        packetToSend += gpsHandler.readGPS();

        rfm95Handler.sendMessage(packetToSend);

        // Reset counter and timer
        loopCounter = 0;
        lastTime = millis();
    }

    gpsHandler.update();
}

void serialEvent() {
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        inputString += inChar;
        if (inChar == '\n') {
            stringComplete = true;
        }
    }
}

float altitudeBuffer[ALTITUDE_BUFFER_SIZE];
int altitudeBufferIndex = 0;

void addAltitudeToBuffer(float altitude) {
    altitudeBuffer[altitudeBufferIndex] = altitude;
    altitudeBufferIndex = (altitudeBufferIndex + 1) % ALTITUDE_BUFFER_SIZE;
}

float computeStandardDeviation() {
    float sum = 0.0, mean, standardDeviation = 0.0;

    int i;
    for (i = 0; i < ALTITUDE_BUFFER_SIZE; ++i) {
        sum += altitudeBuffer[i];
    }
    mean = sum / ALTITUDE_BUFFER_SIZE;

    for (i = 0; i < ALTITUDE_BUFFER_SIZE; ++i) {
        standardDeviation += pow(altitudeBuffer[i] - mean, 2);
    }

    return sqrt(standardDeviation / ALTITUDE_BUFFER_SIZE);
}
