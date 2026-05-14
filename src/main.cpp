#include <Arduino.h>
#include "config.h"
#include "battery_management.h"
#include "display_oled.h"
#include "input_button.h"
#include "sensor_mpu6050.h"
#include "power_management.h"

// Variável preservada durante o Deep Sleep
RTC_DATA_ATTR OperationMode currentMode = MONITORING;

static unsigned long lastDisplayTime = 0;
static unsigned long last_interval_us = 0;
static bool isMenuOpen = false;
static OperationMode menuSelectedMode = MONITORING;

// Helper functions for button press status and waveform rendering
extern bool sampleAccelerometer();
extern float *getWaveBufferX();
extern float *getWaveBufferY();
extern float *getWaveBufferZ();
extern int *getWaveIndex();
extern unsigned long getButtonPressTimeRemaining(unsigned long currentMillis);
extern bool checkSelectorButton(unsigned long currentMillis);
extern bool wasShortClickDetected();
extern bool isButtonPressActive();
extern void drawSelectionMenu(int selectedMode);
extern void drawBigCenteredText(const char *text);
extern void drawDataTransferAnimation(unsigned long currentMillis);

void renderDisplayFrame(unsigned long currentMillis);

void setup()
{
  Serial.begin(115200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0); // Ensure LED is off using the correct driver

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

  // Task 1: Gestão de Botões e Estado
  checkSleepButton(currentMillis);

  if (checkSelectorButton(currentMillis))
  {
    if (!isMenuOpen)
    {
      isMenuOpen = true;
      menuSelectedMode = currentMode;
    }
    else
    {
      menuSelectedMode = (menuSelectedMode == MONITORING) ? DATA_COLLECTION : MONITORING;
    }
    Serial.printf("Menu: Selecionado %s\n", (menuSelectedMode == MONITORING) ? "MONITORAR" : "COLETAR");
  }

  if (wasShortClickDetected())
  {
    if (isMenuOpen)
    {
      currentMode = menuSelectedMode;
      isMenuOpen = false;
      Serial.printf("Modo CONFIRMADO: %s\n", (currentMode == MONITORING) ? "MONITORAMENTO" : "COLETA");
    }
    else
    {
      Serial.printf("Status Atual: Modo %s | Bateria: %d%%\n",
                    (currentMode == MONITORING) ? "Monitoramento" : "Coleta",
                    getBatteryPercentage());
    }
  }

  // Task 2 & 3: Captura e Processamento de Dados (1 kHz interno ao módulo)
  bool hasNewData = sampleAccelerometer();

  if (currentMode == MONITORING)
  {
    if (hasNewData)
    {
      // Aqui entram as funções de TinyML e análise FFT (Manutenção Preditiva)
    }
  }
  else if (hasNewData)
  {
    // Modo COLETA: Transmissão bruta via Serial (Lógica migrada do new.cpp)
    float *x = getWaveBufferX();
    float *y = getWaveBufferY();
    float *z = getWaveBufferZ();
    int lastIdx = (*getWaveIndex() - 1 + WAVE_BUFFER_SIZE) % WAVE_BUFFER_SIZE;
    Serial.printf("%.2f,%.2f,%.2f\n", x[lastIdx], y[lastIdx], z[lastIdx]);
  }

  // Task 4: Update display at lower priority (10 Hz)
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

  // Draw header text when not in long press sleep countdown
  if (isButtonPressActive())
  {
    unsigned long remainingMs = getButtonPressTimeRemaining(currentMillis);
    drawSleepCountdown(remainingMs);
    if (currentMode == MONITORING)
      drawWaveformAxes();
  }
  else if (isMenuOpen)
  {
    drawSelectionMenu(menuSelectedMode);
  }
  else
  {
    drawStatusText(currentMode == MONITORING ? "ANALISE" : "COLETA");

    if (currentMode == MONITORING)
    {
      drawWaveformAxes();
    }
    else
    {
      drawDataTransferAnimation(currentMillis);
    }
  }

  // Exibe o estado da máquina de estados no rodapé, apenas se o menu não estiver aberto
  if (!isMenuOpen)
  {
    // Animação dos 3 pontos: muda a cada 500ms (ciclo de 0 a 3 pontos)
    int numDots = (currentMillis / 500) % 4;
    char statusMsg[20];

    if (currentMode == MONITORING)
    {
      snprintf(statusMsg, sizeof(statusMsg), "PROCESSANDO%.*s", numDots, "...");
    }
    else
    {

      snprintf(statusMsg, sizeof(statusMsg), "ENVIANDO%.*s", numDots, "...");
    }
    drawFooterStatus(statusMsg);
  }

  // Update display
  updateDisplay();
}
