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
  display.setCursor(0, 16);
  display.println(text);
}

void drawSleepCountdown(unsigned long remainingMs)
{
  display.setCursor(0, 16);
  display.println("Segure 5s para dormir");
  display.print("Faltam: ");
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

void drawSleepMessage()
{
  display.setCursor(0, 0);
  display.println("Acordar com long push");
  display.println("Pressione o botao");
  display.println("para acordar");
}

void drawWaveform(int waveOffset)
{
  float* buffer = getWaveBuffer();
  int* index = getWaveIndex();

  for (int i = 0; i < WAVE_BUFFER_SIZE - 1; i++)
  {
    int idx1 = (*index + i) % WAVE_BUFFER_SIZE;
    int idx2 = (*index + i + 1) % WAVE_BUFFER_SIZE;
    int x1 = map(i, 0, WAVE_BUFFER_SIZE - 1, 0, SCREEN_WIDTH - 1);
    int y1 = map(buffer[idx1], -16, 16, SCREEN_HEIGHT - 1, waveOffset);
    int x2 = map(i + 1, 0, WAVE_BUFFER_SIZE - 1, 0, SCREEN_WIDTH - 1);
    int y2 = map(buffer[idx2], -16, 16, SCREEN_HEIGHT - 1, waveOffset);
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }
}

void updateDisplay()
{
  display.display();
}
