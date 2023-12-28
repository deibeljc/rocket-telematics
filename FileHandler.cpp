#include "FileHandler.h"

FileHandler::FileHandler(FatFileSystem &fs, const char *filename) : fatfs(fs), filePath(filename) {}

void FileHandler::setupFile() {
    if (!fatfs.exists(filePath)) {
        File32 file = fatfs.open(filePath, FILE_WRITE | O_CREAT | O_TRUNC);
        if (!file) {
            Serial.println("Failed to create file, " + String(filePath));
            return;
        }
        file.close();
    }
}

// Assuming the total flash memory size in bytes (2MB)
const uint32_t TOTAL_FLASH_MEMORY = 2048 * 1024;

void FileHandler::printFileSize() {
    File32 file = fatfs.open(filePath, FILE_READ);
    if (file) {
        uint32_t fileSize = file.size();
        Serial.print("Size of ");
        Serial.print(filePath);
        Serial.print(" is ");
        Serial.print(fileSize);
        Serial.println(" bytes.");
        file.close();

        uint32_t usedSpace = fileSize; // Assuming only this file occupies space
        uint32_t remainingSpace = TOTAL_FLASH_MEMORY - usedSpace;

        Serial.print("Used flash space: ");
        Serial.print(usedSpace);
        Serial.println(" bytes.");

        Serial.print("Remaining flash space: ");
        Serial.print(remainingSpace);
        Serial.println(" bytes.");
    } else {
        Serial.println("Error opening file to check size");
    }
}

void FileHandler::addToBatch(const String& data) {
    altitudeBatch[batchIndex++] = data;

    if (batchIndex == BATCH_SIZE) {
        writeBatchToFile();
    }
}

void FileHandler::writeBatchToFile() {
  if (batchIndex > 0) {
    File32 dataFile = fatfs.open(filePath, FILE_WRITE);
    if (dataFile) {
        for (unsigned int i = 0; i < BATCH_SIZE; i++) {
            bool wroteSuccessfully = dataFile.println(altitudeBatch[i]);

            if (!wroteSuccessfully) {
              Serial.println("Error writing to file");
            }
        }
        dataFile.close();
        batchIndex = 0;
    } else {
        Serial.println("Error opening file for writing");
    }
  }
}

void FileHandler::deleteFile() {
    if (fatfs.remove(filePath)) {
        Serial.println("File deleted successfully");
    } else {
        Serial.println("Error deleting file");
    }
}

void FileHandler::sendFileDataOverSerial() {
    File32 dataFile = fatfs.open(filePath, FILE_READ);
    if (dataFile) {
        Serial.println("Start");
        // Add a header for our CSV.
        Serial.println("millis,temp,acc_x,acc_y,acc_z,gyro_x,gyro_y,gyro_z,altitude,lat,long");
        unsigned long lineCount = 0;
        while (dataFile.available()) {
            char ch = dataFile.read();
            Serial.write(ch);
            if (ch == '\n') {
                lineCount++;
            }
        }
        dataFile.close();
        Serial.println("End");
        Serial.print("\nEnd of file. Total lines: ");
        Serial.println(lineCount);
    } else {
        Serial.println("Error opening file for reading");
    }
}
