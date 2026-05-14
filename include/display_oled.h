#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

// Initialize OLED display
void initDisplay();

// Clear and prepare display for rendering
void clearDisplay();

// Draw battery indicator at top-right corner
void drawBatteryIndicator();

// Draw text: "EVA - Monitorando..."
void drawStatusText(const char* text);

// Draw sleep countdown text and remaining time
void drawSleepCountdown(unsigned long remainingMs);

// Draw wake-up countdown (for handleWakeFromSleep)
void drawWakeCountdown(unsigned long remainingMs);

// Draw status text at the display footer
void drawFooterStatus(const char* status);

// Draw the acceleration waveform for all MPU axes
void drawWaveformAxes();

// Draw a single axis waveform within a defined display window
void drawWaveformForAxis(float* buffer, int top, int bottom, const char* label);

// Update display on screen
void updateDisplay();

// Draw "Entering sleep" message
void drawSleepMessage();

#endif // DISPLAY_OLED_H
