#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "config.h"
#include "display_oled.h"
#include "battery_management.h"
#include "sensor_mpu6050.h"

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void initDisplay()
{
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    while (true)
    {
      delay(10);
    }
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

  // Display percentage text
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x - 20, y);
  display.print(batteryPercent);
  display.print("%");
}

void drawStatusText(const char* text)
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
  display.setCursor(0, 0);
  display.println("Acordar: mantenha");
  display.print("Faltam: ");
  display.print((remainingMs + 999) / 1000);
  display.println(" s");
}

void drawFooterStatus(const char* status)
{
  display.setCursor(0, SCREEN_HEIGHT - 8);
  display.print("STATUS: ");
  display.print(status);
}

void drawSleepMessage()
{
  display.setCursor(0, 0);
  display.println("Acordar com long push");
  display.println("Pressione o botao");
  display.println("para acordar");
}

void drawWaveformForAxis(float* buffer, int top, int bottom, const char* label)
{
  int* index = getWaveIndex();

  display.setCursor(0, top);
  display.print(label);

  for (int i = 0; i < WAVE_BUFFER_SIZE - 1; i++)
  {
    int idx1 = (*index + i) % WAVE_BUFFER_SIZE;
    int idx2 = (*index + i + 1) % WAVE_BUFFER_SIZE;
    int x1 = map(i, 0, WAVE_BUFFER_SIZE - 1, 0, SCREEN_WIDTH - 1);
    int y1 = map(buffer[idx1], -16, 16, bottom, top);
    int x2 = map(i + 1, 0, WAVE_BUFFER_SIZE - 1, 0, SCREEN_WIDTH - 1);
    int y2 = map(buffer[idx2], -16, 16, bottom, top);
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }
}

void drawWaveformAxes()
{
  const int topMargin = 18;
  const int axisHeight = 10;
  const int axisSpacing = 2;

  drawWaveformForAxis(getWaveBufferX(), topMargin, topMargin + axisHeight, "X");
  drawWaveformForAxis(getWaveBufferY(), topMargin + axisHeight + axisSpacing, topMargin + 2 * axisHeight + axisSpacing, "Y");
  drawWaveformForAxis(getWaveBufferZ(), topMargin + 2 * (axisHeight + axisSpacing), topMargin + 3 * axisHeight + 2 * axisSpacing, "Z");
}

void updateDisplay()
{
  display.display();
}
