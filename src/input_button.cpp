#include <Arduino.h>
#include "config.h"
#include "input_button.h"
#include "power_management.h"

static unsigned long buttonPressStart = 0;
static bool shortClickPending = false;

// Variáveis para o seletor (Pino 5)
static unsigned long lastSelectorDebounce = 0;
static bool lastSelectorState = HIGH;

void initButton()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SELECTOR_BUTTON_PIN, INPUT_PULLUP); // Botão Seletor de Modo
}

bool isButtonPressed()
{
  return digitalRead(BUTTON_PIN) == LOW;
}

bool checkSelectorButton(unsigned long currentMillis)
{
  bool reading = digitalRead(SELECTOR_BUTTON_PIN);
  bool triggered = false;

  if (reading != lastSelectorState) {
    if ((currentMillis - lastSelectorDebounce) > DEBOUNCE_DELAY_MS) {
      if (reading == LOW) { // Pressionado
        triggered = true;
      }
      lastSelectorState = reading;
      lastSelectorDebounce = currentMillis;
    }
  }
  return triggered;
}

bool wasShortClickDetected() {
  bool status = shortClickPending;
  shortClickPending = false;
  return status;
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
    // Se soltou o botão antes do tempo de Deep Sleep, é um clique curto
    if (buttonPressStart != 0 && (currentMillis - buttonPressStart < LONG_PRESS_MS)) {
      shortClickPending = true;
    }
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
