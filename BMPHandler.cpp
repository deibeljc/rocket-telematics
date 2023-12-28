#include "BMPHandler.h"
#include <Wire.h>

BMPHandler::BMPHandler() {}

bool BMPHandler::setup() {
    if (!bmp.begin_SPI(BMP_CS)) {
        return false;
    }
    bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    bmp.setOutputDataRate(BMP3_ODR_100_HZ);
    return true;
}

float BMPHandler::readAltitude() {
    if (bmp.performReading()) {
        return bmp.readAltitude(SEALEVELPRESSURE_HPA);
    }
    return -1; // Indicate a failed reading
}
