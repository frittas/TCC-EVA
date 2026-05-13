#ifndef INPUT_BUTTON_H
#define INPUT_BUTTON_H

// Initialize button pin
void initButton();

// Check if button is currently pressed
bool isButtonPressed();

// Handle button long-press for sleep
void checkSleepButton(unsigned long currentMillis);

// Reset button press tracking
void resetButtonState();

// Get remaining time for button press (for display countdown)
unsigned long getButtonPressTimeRemaining(unsigned long currentMillis);

// Check if button press is active
bool isButtonPressActive();

#endif // INPUT_BUTTON_H
