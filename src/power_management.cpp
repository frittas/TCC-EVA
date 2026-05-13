#include <Arduino.h>
#include <esp_sleep.h>
#include "config.h"
#include "power_management.h"
#include "display_oled.h"
#include "input_button.h"

void handleWakeFromSleep()
{
  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_EXT0)
  {
    return;
  }

  unsigned long wakeStart = millis();
  while (isButtonPressed() && millis() - wakeStart < LONG_PRESS_MS)
  {
    unsigned long remaining = (wakeStart + LONG_PRESS_MS) - millis();
    
    clearDisplay();
    drawWakeCountdown(remaining);
    updateDisplay();
    delay(100);
  }

  if (isButtonPressed() && millis() - wakeStart >= LONG_PRESS_MS)
  {
    Serial.println("Woke up by long button press");
  }
  else
  {
    Serial.println("Wake button release too early, back to sleep");
    while (isButtonPressed())
    {
      delay(10);
    }
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, 0);
    esp_deep_sleep_start();
  }
}

void enterDeepSleep()
{
  clearDisplay();
  drawSleepMessage();
  updateDisplay();

  Serial.println("Entering deep sleep...");

  // Aguarda soltar o botao antes de entrar em deep sleep
  while (isButtonPressed())
  {
    delay(10);
  }

  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, 0);
  esp_deep_sleep_start();
}
