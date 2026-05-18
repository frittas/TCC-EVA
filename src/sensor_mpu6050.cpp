#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "config.h"
#include "sensor_mpu6050.h"

static Adafruit_MPU6050 mpu;
static float waveBufferX[WAVE_BUFFER_SIZE];
static float waveBufferY[WAVE_BUFFER_SIZE];
static float waveBufferZ[WAVE_BUFFER_SIZE];
static int waveIndex = 0;
static unsigned long lastSampleTime = 0;

static bool mpuInitialized = false;

void initMPU6050()
{
  if (!mpu.begin(0x68))
  {
    Serial.println(F("Falha ao inicializar MPU6050!"));
    Serial.println(F("Verifique a conexão I2C e o endereço do sensor."));
    neopixelWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0); // Red
    mpuInitialized = false;
    return;
  }

  mpuInitialized = true;
  neopixelWrite(RGB_BUILTIN, 0, 0, RGB_BRIGHTNESS); // Blue
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

bool sampleAccelerometer()
{
  if (!mpuInitialized)
  {
    return false;
  }
  unsigned long currentMicros = micros();

  if (currentMicros - lastSampleTime >= SAMPLE_INTERVAL)
  {
    lastSampleTime = currentMicros;

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    waveBufferX[waveIndex] = a.acceleration.x;
    waveBufferY[waveIndex] = a.acceleration.y;
    waveBufferZ[waveIndex] = a.acceleration.z;

    waveIndex = (waveIndex + 1) % WAVE_BUFFER_SIZE;
    return true;
  }
  return false;
}

float* getWaveBufferX()
{
  return waveBufferX;
}

float* getWaveBufferY()
{
  return waveBufferY;
}

float* getWaveBufferZ()
{
  return waveBufferZ;
}

int* getWaveIndex()
{
  return &waveIndex;
}
