#ifndef SENSOR_MPU6050_H
#define SENSOR_MPU6050_H

// Initialize MPU6050 sensor
void initMPU6050();

// Sample accelerometer data into waveform buffers
bool sampleAccelerometer();

// Get direct reference to waveform buffers and index
float* getWaveBufferX();
float* getWaveBufferY();
float* getWaveBufferZ();
int* getWaveIndex();

#endif // SENSOR_MPU6050_H
