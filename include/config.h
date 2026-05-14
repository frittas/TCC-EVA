#ifndef CONFIG_H
#define CONFIG_H

// Operation Modes
enum OperationMode { MONITORING, DATA_COLLECTION };

// I2C Configuration
#define I2C_SDA 8
#define I2C_SCL 9

// Button GPIO Pins
#define BUTTON_PIN 4            // Wake-up and Confirmation Button
#define SELECTOR_BUTTON_PIN 5   // Mode Selector Button

// LED Configuration
#define RGB_BRIGHTNESS 64       // Default brightness for built-in RGB LED (0-255)

// Button Timing and Debounce
#define LONG_PRESS_MS 3000      // Time required to enter deep sleep
#define DEBOUNCE_DELAY_MS 50    // Debounce time for selector button

// Battery Management
#define BATTERY_PIN 1
#define ADC_MAX 4095
#define ADC_VOLTAGE_REFERENCE 3.3
#define BATTERY_MAX_VOLTAGE 4.2
#define BATTERY_MIN_VOLTAGE 3.0

// Display Configuration
#define OLED_ADDRESS 0x3C
#define OLED_RESET -1
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define DISPLAY_INTERVAL 100    // 100ms = 10Hz

// Sensor Configuration
#define WAVE_BUFFER_SIZE 128
#define SAMPLE_INTERVAL 1000    // 1000us = 1kHz

#endif // CONFIG_H
