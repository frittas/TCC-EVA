#ifndef CONFIG_H
#define CONFIG_H

// I2C Pins
#define I2C_SDA 8
#define I2C_SCL 9

// GPIO Pins
#define BUTTON_PIN 4
#define BATTERY_PIN 1

// Button behavior
#define LONG_PRESS_MS 3000

// Battery parameters (LiPo)
#define BATTERY_MAX_VOLTAGE 4.2
#define BATTERY_MIN_VOLTAGE 3.0
#define ADC_MAX 4095
#define ADC_VOLTAGE_REFERENCE 3.3

// OLED Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

// Waveform sampling
#define WAVE_BUFFER_SIZE 128
#define SAMPLE_INTERVAL 1000  // microseconds = 1 kHz
#define DISPLAY_INTERVAL 100  // milliseconds = 10 Hz

#endif // CONFIG_H
