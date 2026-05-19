#include <Arduino.h>
#include "config.h"
#include "battery_management.h"
#include "display_oled.h"
#include "input_button.h"
#include "sensor_mpu6050.h"
#include "power_management.h"
#include "mqtt.h"
#include "wifi_management.h"

// Variável preservada durante o Deep Sleep
RTC_DATA_ATTR OperationMode currentMode = MONITORING;

static unsigned long lastDisplayTime = 0;
static unsigned long last_interval_us = 0;
static bool isMenuOpen = false;
static OperationMode menuSelectedMode = MONITORING;

void setup()
{
  Serial.begin(115200);
  delay(100);

#if DEBUG_MODE
  Serial.println(F("=== DEBUG MODE ATIVO ==="));
  Serial.printf("Build: %s %s\n", __DATE__, __TIME__);
  Serial.printf("ESP32 SDK version: %s\n", ESP.getSdkVersion());
  Serial.printf("Heap inicial: %u bytes\n", ESP.getFreeHeap());
#else
  neopixelWrite(RGB_BUILTIN, 0, 0, 0); // Ensure LED is off using the correct driver
  Serial.println(F("=== PRODUCTION MODE ATIVO ==="));
  Serial.printf("Build: %s %s\n", __DATE__, __TIME__);
#endif

  // Initialize all modules
#if DEBUG_MODE
  Serial.println(F("Inicializando modulo de bateria..."));
#endif
  initBattery();
#if DEBUG_MODE
  Serial.println(F("Inicializando modulo de botao..."));
#endif
  initButton();
#if DEBUG_MODE
  Serial.println(F("Inicializando display..."));
#endif
  initDisplay();
#if DEBUG_MODE
  Serial.println(F("Inicializando MPU6050..."));
#endif
  initMPU6050();
#if DEBUG_MODE
  Serial.println(F("Inicializando WiFi..."));
#endif
  initWiFi();
#if DEBUG_MODE
  Serial.println(F("Inicializando MQTT..."));
#endif
  initMQTT(); // Inicializa conexão MQTT com ThingsBoard

  // Handle wake from deep sleep if applicable
#if DEBUG_MODE
  Serial.println(F("Verificando wake from sleep..."));
#endif
  handleWakeFromSleep();

#if DEBUG_MODE
  Serial.println(F("Setup concluido. Entrando no loop."));
  Serial.printf("Heap disponivel: %u bytes\n", ESP.getFreeHeap());
#endif

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
      // EXEMPLO: Simular resultado de inferência para enviar via MQTT
      float *x = getWaveBufferX();
      float *y = getWaveBufferY();
      float *z = getWaveBufferZ();
      int bufferSize = WAVE_BUFFER_SIZE;

      // Simulação: resultado da inferência de ML
      String statusPredito = "Saudável"; // Resultado do TinyML
      float confianca = 0.95f;           // Confiança do modelo (0-1)

      // Envia dados para MQTT (armazena para envio a cada 5s)
      sendData(statusPredito, confianca, x, y, z, bufferSize);
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

  // Task 4: Update MQTT (envio periódico a cada 5 segundos)
  updateMQTT();

  // Task 5: Update display at lower priority (10 Hz)
  if (currentMillis - lastDisplayTime >= DISPLAY_INTERVAL)
  {
    lastDisplayTime = currentMillis;
    renderDisplayFrame(currentMillis, currentMode, isMenuOpen, menuSelectedMode);
  }
  // Serial.println("Loop principal rodando... (Descomente o código para ativar as funcionalidades)"); // Placeholder
}
