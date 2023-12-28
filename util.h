#ifndef UTIL_H
#define UTIL_H

#define SMOOTHING_WINDOW_SIZE 20
#define ALTITUDE_BUFFER_SIZE 10

float calculateMedian(float arr[], int n);
void removeOutliers(float arr[], int &n);
float calculateMovingAverage(float newAltitude);
float computeStandardDeviation();

extern float runningSum;
extern float runningSumOfSquares;

#endif
