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

// Draw the acceleration waveform
void drawWaveform(int waveOffset);

// Update display on screen
void updateDisplay();

// Draw "Entering sleep" message
void drawSleepMessage();

#endif // DISPLAY_OLED_H
