#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <SdFat.h>
#include <Adafruit_SPIFlash.h>

#define BATCH_SIZE 100

class FileHandler {
public:
    FileHandler(FatFileSystem &fs, const char *filename);
    void setupFile();
    void addToBatch(const String& data);
    void writeBatchToFile();
    void deleteFile();
    void sendFileDataOverSerial();
    void printFileSize();

private:
    FatFileSystem &fatfs;
    const char *filePath;
    String altitudeBatch[BATCH_SIZE];
    unsigned int batchIndex = 0;
};

#endif
