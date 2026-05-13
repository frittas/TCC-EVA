#include <Arduino.h>
#include "config.h"
#include "input_button.h"
#include "power_management.h"

static unsigned long buttonPressStart = 0;

void initButton()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

bool isButtonPressed()
{
  return digitalRead(BUTTON_PIN) == LOW;
}

void checkSleepButton(unsigned long currentMillis)
{
  bool buttonPressed = isButtonPressed();

  if (buttonPressed)
  {
    if (buttonPressStart == 0)
    {
      buttonPressStart = currentMillis;
    }
    else if (currentMillis - buttonPressStart >= LONG_PRESS_MS)
    {
      enterDeepSleep();
    }
  }
  else
  {
    buttonPressStart = 0;
  }
}

void resetButtonState()
{
  buttonPressStart = 0;
}

unsigned long getButtonPressTimeRemaining(unsigned long currentMillis)
{
  if (buttonPressStart == 0) return LONG_PRESS_MS;
  unsigned long elapsed = currentMillis - buttonPressStart;
  return (elapsed >= LONG_PRESS_MS) ? 0 : (LONG_PRESS_MS - elapsed);
}

bool isButtonPressActive()
{
  return buttonPressStart != 0;
}
