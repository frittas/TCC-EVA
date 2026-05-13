#include <Arduino.h>
#include "config.h"
#include "battery_management.h"
#include "display_oled.h"
#include "input_button.h"
#include "sensor_mpu6050.h"
#include "power_management.h"

static unsigned long lastDisplayTime = 0;

// Helper functions for button press status and waveform rendering
extern unsigned long getButtonPressTimeRemaining(unsigned long currentMillis);
extern bool isButtonPressActive();

void renderDisplayFrame(unsigned long currentMillis);

void setup()
{
  Serial.begin(115200);
  
  // Initialize all modules
  initBattery();
  initButton();
  initDisplay();
  initMPU6050();
  
  // Handle wake from deep sleep if applicable
  handleWakeFromSleep();
  
  delay(100);
}

void loop()
{
  unsigned long currentMicros = micros();
  unsigned long currentMillis = millis();

  // Task 1: Check button for long-press sleep
  checkSleepButton(currentMillis);

  // Task 2: Sample accelerometer at high priority (1 kHz)
  sampleAccelerometer();

  // Task 3: Update display at lower priority (10 Hz)
  if (currentMillis - lastDisplayTime >= DISPLAY_INTERVAL)
  {
    lastDisplayTime = currentMillis;
    renderDisplayFrame(currentMillis);
  }
}

void renderDisplayFrame(unsigned long currentMillis)
{
  clearDisplay();
  
  // Draw battery indicator at top-right
  drawBatteryIndicator();
  
  // Draw status or sleep countdown
  if (isButtonPressActive())
  {
    unsigned long remainingMs = getButtonPressTimeRemaining(currentMillis);
    drawSleepCountdown(remainingMs);
  }
  else
  {
    drawStatusText("EVA - Monitorando...");
  }
  
  // Draw waveform
  int waveOffset = isButtonPressActive() ? 34 : 26;
  drawWaveform(waveOffset);
  
  // Update display
  updateDisplay();
}
