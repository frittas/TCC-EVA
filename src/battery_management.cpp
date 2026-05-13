#include <Arduino.h>
#include "config.h"
#include "battery_management.h"

void initBattery()
{
  pinMode(BATTERY_PIN, INPUT);
}

int getBatteryPercentage()
{
  int adcValue = analogRead(BATTERY_PIN);
  float voltage = (adcValue / (float)ADC_MAX) * ADC_VOLTAGE_REFERENCE;

  // Scale voltage to battery percentage
  int percentage = (int)((voltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE) * 100);

  // Constrain to 0-100%
  if (percentage < 0) percentage = 0;
  if (percentage > 100) percentage = 100;

  return percentage;
}
