#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "config.h"
#include <string.h>
#include "display_oled.h"
#include "input_button.h" // Para isButtonPressActive e getButtonPressTimeRemaining
#include "battery_management.h"
#include "sensor_mpu6050.h"
#include "mqtt.h"

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void initDisplay()
{
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    Serial.println(F("Continuando sem display OLED."));
    return;
  }

  display.clearDisplay();
  display.display();
}

void clearDisplay()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void drawBatteryIndicator()
{
  int batteryPercent = getBatteryPercentage();

  // Battery icon position (top-right corner)
  int x = SCREEN_WIDTH - 18;
  int y = 1;
  int width = 16;
  int height = 8;

  // Draw battery outline
  display.drawRect(x, y, width - 2, height, SSD1306_WHITE);
  display.drawRect(x + width - 2, y + 2, 2, height - 4, SSD1306_WHITE);

  // Draw battery fill based on percentage
  int fillWidth = (batteryPercent * (width - 4)) / 100;
  display.fillRect(x + 2, y + 2, fillWidth, height - 4, SSD1306_WHITE);

  display.setFont(); // Garantir fonte padrão
  // Display percentage text
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x - 20, y);
  display.print(batteryPercent);
  display.print("%");
}

void drawStatusText(const char *text)
{
  display.setCursor(0, 0);
  display.println(text);
}

static void drawPowerSymbol(int x, int y)
{
  const int radius = 5;
  display.drawCircle(x + radius, y + radius, radius, SSD1306_WHITE);
  display.drawLine(x + radius, y + 1, x + radius, y + radius + 1, SSD1306_WHITE);
  display.fillCircle(x + radius, y + radius, 1, SSD1306_WHITE);
}

void drawSleepCountdown(unsigned long remainingMs)
{
  drawPowerSymbol(0, 0);
  display.setCursor(14, 0);
  display.print((remainingMs + 999) / 1000);
  display.println(" s");
}

void drawWakeCountdown(unsigned long remainingMs)
{
  const char *name = "E.V.A";
  display.setTextSize(3);
  display.setTextWrap(false); // Impede que o texto "pule" para baixo ao sair da tela

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);

  // Calcula o progresso baseado no tempo decorrido do clique longo
  unsigned long elapsed = (remainingMs > LONG_PRESS_MS) ? 0 : (LONG_PRESS_MS - remainingMs);
  
  // Define o destino (centro) e a origem (fora da tela à direita)
  int16_t targetX = (SCREEN_WIDTH - w) / 2;
  int16_t startX = SCREEN_WIDTH;
  
  int16_t currentX = map((long)elapsed, 0L, (long)LONG_PRESS_MS, (long)startX, (long)targetX);
  int16_t centerY = (SCREEN_HEIGHT - h) / 2;

  display.setCursor(currentX, centerY - y1); // y1 compensa o deslocamento da linha de base
  display.print(name);

  // Timer do Long Press no canto superior direito com o símbolo de Power
  drawPowerSymbol(SCREEN_WIDTH - 25, 0);
  display.setTextSize(1);
  display.setCursor(SCREEN_WIDTH - 12, 1);
  display.print((remainingMs + 999) / 1000);
  
  display.setTextWrap(true); // Restaura o padrão para o resto da interface
}

void drawFooterStatus(const char *status)
{
  int16_t x1, y1;
  uint16_t w, h;

  // Cria uma versão da string com o tamanho máximo (3 pontos) para fixar o X
  char measureText[24];
  strncpy(measureText, status, sizeof(measureText) - 1);
  measureText[sizeof(measureText) - 1] = '\0';

  char *dotPtr = strchr(measureText, '.');
  if (dotPtr)
    *dotPtr = '\0';           // Remove pontos variáveis
  strcat(measureText, "..."); // Simula o comprimento total para medição

  display.getTextBounds(measureText, 0, 0, &x1, &y1, &w, &h);
  int16_t x = (SCREEN_WIDTH - w) / 2;

  display.setCursor(x, SCREEN_HEIGHT - 8);
  display.print(status);
}

void drawSleepMessage()
{
  display.setCursor(0, 0);
  display.println("Acordar com long push");
  display.println("Pressione o botao");
  display.println("para acordar");
}

void drawWaveformForAxis(float *buffer, int top, int bottom, const char *label)
{
  int *index = getWaveIndex();
  const int xOffset = 10; // Margem para não sobrepor a letra do eixo

  display.setCursor(0, top);
  display.print(label);

  for (int i = 0; i < WAVE_BUFFER_SIZE - 1; i++)
  {
    int idx1 = (*index + i) % WAVE_BUFFER_SIZE;
    int idx2 = (*index + i + 1) % WAVE_BUFFER_SIZE;
    int x1 = map(i, 0, WAVE_BUFFER_SIZE - 1, xOffset, SCREEN_WIDTH - 1);
    int y1 = map(buffer[idx1], -16, 16, bottom, top);
    int x2 = map(i + 1, 0, WAVE_BUFFER_SIZE - 1, xOffset, SCREEN_WIDTH - 1);
    int y2 = map(buffer[idx2], -16, 16, bottom, top);
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }
}

// Desenha a forma de onda do RMS triaxial calculado a partir das buffers X/Y/Z
void drawWaveformRMS(int top, int bottom, const char *label)
{
  float *bx = getWaveBufferX();
  float *by = getWaveBufferY();
  float *bz = getWaveBufferZ();
  int *indexPtr = getWaveIndex();
  int idx = *indexPtr;

  // Calcula média (DC) por eixo
  float meanX = 0, meanY = 0, meanZ = 0;
  for (int i = 0; i < WAVE_BUFFER_SIZE; i++)
  {
    meanX += bx[i];
    meanY += by[i];
    meanZ += bz[i];
  }
  meanX /= WAVE_BUFFER_SIZE;
  meanY /= WAVE_BUFFER_SIZE;
  meanZ /= WAVE_BUFFER_SIZE;

  // Cria um buffer temporário de magnitudes AC
  static float magBuf[WAVE_BUFFER_SIZE];
  float maxMag = 0.001f; // evitar divisão por zero
  for (int i = 0; i < WAVE_BUFFER_SIZE; i++)
  {
    int ridx = (idx + i) % WAVE_BUFFER_SIZE;
    float acX = bx[ridx] - meanX;
    float acY = by[ridx] - meanY;
    float acZ = bz[ridx] - meanZ;
    float mag = sqrt(acX * acX + acY * acY + acZ * acZ);
    magBuf[i] = mag;
    if (mag > maxMag) maxMag = mag;
  }

  // Desenha label
  display.setCursor(0, top);
  display.print(label);

  // Mapeia e desenha a linha
  for (int i = 0; i < WAVE_BUFFER_SIZE - 1; i++)
  {
    int x1 = map(i, 0, WAVE_BUFFER_SIZE - 1, 10, SCREEN_WIDTH - 1);
    int x2 = map(i + 1, 0, WAVE_BUFFER_SIZE - 1, 10, SCREEN_WIDTH - 1);
    // Map magnitude to vertical position (bottom..top)
    float v1 = magBuf[i] / maxMag; // 0..1
    float v2 = magBuf[i + 1] / maxMag;
    int y1 = bottom - (int)((bottom - top - 4) * v1); // leave small padding
    int y2 = bottom - (int)((bottom - top - 4) * v2);
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }
}

void drawWaveformAxes()
{
  // Substitui exibição dos eixos por RMS triaxial
  const int topMargin = 18;
  const int waveHeight = 28; // ocupar área maior
  drawWaveformRMS(topMargin, topMargin + waveHeight, "RMS");
}

void drawSelectionMenu(int selectedMode)
{
  display.setTextSize(1);
  display.setCursor(15, 18);
  display.println(F("SELECIONE O MODO:"));

  display.setCursor(20, 32);
  display.print(selectedMode == 0 ? F("> MONITORAR") : F("  MONITORAR"));

  display.setCursor(20, 42);
  display.print(selectedMode == 1 ? F("> COLETAR") : F("  COLETAR"));
}

void drawBigCenteredText(const char *text)
{
  display.setTextSize(2);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT - h) / 2);
  display.print(text);
  display.setTextSize(1);
}

void drawDataTransferAnimation(unsigned long currentMillis)
{
  int espX = 20;
  int cloudX = SCREEN_WIDTH - 25;
  int centerY = 35; // Abaixo da faixa amarela (Y=15)

  // 1. Desenha Símbolo USB "Trident" na Horizontal (Esquerda)
  display.fillCircle(espX - 10, centerY, 2, SSD1306_WHITE); // Base
  display.drawLine(espX - 8, centerY, espX + 6, centerY, SSD1306_WHITE); // Tronco
  display.fillTriangle(espX + 11, centerY, espX + 6, centerY - 3, espX + 6, centerY + 3, SSD1306_WHITE); // Seta
  // Ramo Superior (Quadrado)
  display.drawLine(espX - 3, centerY, espX + 1, centerY - 5, SSD1306_WHITE);
  display.fillRect(espX + 1, centerY - 7, 3, 3, SSD1306_WHITE);
  // Ramo Inferior (Círculo)
  display.drawLine(espX - 3, centerY, espX + 1, centerY + 5, SSD1306_WHITE);
  display.fillCircle(espX + 3, centerY + 5, 2, SSD1306_WHITE);

  // 2. Desenha Nuvem Estilizada (Direita)
  display.fillCircle(cloudX - 10, centerY + 4, 5, SSD1306_WHITE); // Esquerda
  display.fillCircle(cloudX + 10, centerY + 4, 5, SSD1306_WHITE); // Direita
  display.fillCircle(cloudX - 4, centerY, 7, SSD1306_WHITE);      // Centro-Esquerda
  display.fillCircle(cloudX + 5, centerY + 1, 6, SSD1306_WHITE);  // Centro-Direita
  display.fillRect(cloudX - 10, centerY + 4, 21, 5, SSD1306_WHITE); // Base reta

  // 3. Animação de setas na Horizontal
  const int animationStart = espX + 16;
  const int animationEnd = cloudX - 15;

  for (int i = 0; i < 2; i++)
  {
    int progress = ((currentMillis + (i * 600)) % 1200);
    int tipX = map(progress, 0, 1200, animationStart, animationEnd);

    if (tipX < animationStart + 2)
      continue;

    display.drawLine(tipX - 5, centerY, tipX, centerY, SSD1306_WHITE);     // Corpo
    display.drawLine(tipX, centerY, tipX - 2, centerY - 2, SSD1306_WHITE); // Ponta cima
    display.drawLine(tipX, centerY, tipX - 2, centerY + 2, SSD1306_WHITE); // Ponta baixo
  }
}

void updateDisplay()
{
  display.display();
}

void renderDisplayFrame(unsigned long currentMillis, OperationMode currentMode, bool isMenuOpen, OperationMode menuSelectedMode)
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
      const char *base = isMQTTConnected() ? "MQTT:ONLINE" : "MQTT:OFFLINE";
      snprintf(statusMsg, sizeof(statusMsg), "%s%.*s", base, numDots, "...");
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
