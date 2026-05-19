#include <Arduino.h>
#include <arduinoFFT.h>
#include <math.h>
#include "fft_analysis.h"

static arduinoFFT FFT = arduinoFFT();

void calculateTriaxialFFT(float *bufferX,
                           float *bufferY,
                           float *bufferZ,
                           int numSamples,
                           int currentIndex,
                           float *magnitudeBins,
                           int numBins)
{
  int windowSize = min(numSamples, FFT_SAMPLE_WINDOW);
  int startIndex = (currentIndex - windowSize + numSamples) % numSamples;

  // Remove componente DC e obtém o sinal triaxial
  double vReal[FFT_SAMPLE_WINDOW];
  double vImag[FFT_SAMPLE_WINDOW];

  float meanX = 0.0f;
  float meanY = 0.0f;
  float meanZ = 0.0f;
  for (int i = 0; i < windowSize; i++)
  {
    int index = (startIndex + i) % numSamples;
    meanX += bufferX[index];
    meanY += bufferY[index];
    meanZ += bufferZ[index];
  }
  meanX /= windowSize;
  meanY /= windowSize;
  meanZ /= windowSize;

  for (int i = 0; i < windowSize; i++)
  {
    int index = (startIndex + i) % numSamples;
    float acX = bufferX[index] - meanX;
    float acY = bufferY[index] - meanY;
    float acZ = bufferZ[index] - meanZ;
    vReal[i] = sqrt(acX * acX + acY * acY + acZ * acZ);
    vImag[i] = 0.0;
  }

  FFT.Windowing(vReal, windowSize, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, windowSize, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, windowSize);

  int fullBins = windowSize / 2;
  int binsPerDisplay = max(1, fullBins / numBins);

  for (int bin = 0; bin < numBins; bin++)
  {
    double sum = 0.0;
    int count = 0;
    for (int j = 0; j < binsPerDisplay; j++)
    {
      int srcBin = bin * binsPerDisplay + j;
      if (srcBin >= fullBins)
        break;
      sum += vReal[srcBin];
      count++;
    }

    magnitudeBins[bin] = (count > 0) ? (float)(sum / count) : 0.0f;
  }
}
