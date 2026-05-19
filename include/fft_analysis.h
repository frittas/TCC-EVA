#ifndef FFT_ANALYSIS_H
#define FFT_ANALYSIS_H

#include "config.h"

// Calcula a FFT triaxial a partir das buffers de aceleração X/Y/Z.
// currentIndex é o próximo índice a ser escrito no buffer circular.
// magnitudeBins deve ter tamanho numBins.
void calculateTriaxialFFT(float *bufferX,
                           float *bufferY,
                           float *bufferZ,
                           int numSamples,
                           int currentIndex,
                           float *magnitudeBins,
                           int numBins);

#endif // FFT_ANALYSIS_H
