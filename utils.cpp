#include "util.h"
#include <math.h>

extern float altitudeBuffer[];
extern float altitudeWindow[];
extern int altitudeWindowIndex;
extern bool altitudeWindowFull;

float calculateMedian(float arr[], int n) {
  if (n % 2 == 0)
    return (arr[n / 2 - 1] + arr[n / 2]) / 2.0;
  else
    return arr[n / 2];
}

void quicksort(float arr[], int left, int right) {
  int i = left, j = right;
  float pivot = arr[(left + right) / 2];
  while (i <= j) {
    while (arr[i] < pivot) i++;
    while (arr[j] > pivot) j--;
    if (i <= j) {
      float temp = arr[i];
      arr[i] = arr[j];
      arr[j] = temp;
      i++;
      j--;
    }
  }
  if (left < j) quicksort(arr, left, j);
  if (i < right) quicksort(arr, i, right);
}

void removeOutliers(float arr[], int &n) {
  // Sort the array using quicksort for better performance
  quicksort(arr, 0, n - 1);

  // Calculate Q1 and Q3
  float Q1 = calculateMedian(arr, n / 2);
  float Q3 = calculateMedian(arr + (n + 1) / 2, n / 2);

  // Calculate IQR
  float IQR = Q3 - Q1;

  // Define acceptable range
  float lowerBound = Q1 - 1.5 * IQR;
  float upperBound = Q3 + 1.5 * IQR;

  // Filter out outliers
  int newSize = 0;
  for (int i = 0; i < n; i++) {
    if (arr[i] >= lowerBound && arr[i] <= upperBound) {
      arr[newSize++] = arr[i];
    }
  }
  n = newSize;
}


float calculateMovingAverage(float newAltitude) {
  if (altitudeWindowFull) {
    float oldAltitude = altitudeWindow[altitudeWindowIndex];
    runningSum -= oldAltitude;
    runningSumOfSquares -= oldAltitude * oldAltitude;
  }

  altitudeWindow[altitudeWindowIndex] = newAltitude;
  runningSum += newAltitude;
  runningSumOfSquares += newAltitude * newAltitude;
  altitudeWindowIndex = (altitudeWindowIndex + 1) % SMOOTHING_WINDOW_SIZE;

  if (altitudeWindowIndex == 0) {
    altitudeWindowFull = true;
  }

  int count = altitudeWindowFull ? SMOOTHING_WINDOW_SIZE : altitudeWindowIndex;
  return runningSum / count;
}


float computeStandardDeviation() {
  int count = altitudeWindowFull ? SMOOTHING_WINDOW_SIZE : altitudeWindowIndex;
  if (count == 0) return 0.0;

  float mean = runningSum / count;
  float variance = (runningSumOfSquares / count) - (mean * mean);
  return sqrt(variance);
}
