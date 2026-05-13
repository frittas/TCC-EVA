#ifndef SENSOR_MPU6050_H
#define SENSOR_MPU6050_H

// Initialize MPU6050 sensor
void initMPU6050();

// Sample accelerometer data into waveform buffer
void sampleAccelerometer();

// Get direct reference to waveform buffer and index
float* getWaveBuffer();
int* getWaveIndex();

#endif // SENSOR_MPU6050_H
