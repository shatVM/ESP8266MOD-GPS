#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

// Оголошуємо тип для функції зворотного виклику (callback)
typedef void (*ButtonActionCallback)();

// Глобальні змінні для модуля
namespace ButtonHandler {
    extern ButtonActionCallback actionCallback;
    // volatile, оскільки змінна змінюється в перериванні
    volatile bool buttonPressed = false; 
    volatile unsigned long lastInterruptTime = 0;
    const unsigned long debounceTime = 250; // 250 мс для усунення брязкоту
}

// Функція, що буде викликатися перериванням
ICACHE_RAM_ATTR void handleInterrupt() {
    if (millis() - ButtonHandler::lastInterruptTime > ButtonHandler::debounceTime) {
        ButtonHandler::buttonPressed = true;
        ButtonHandler::lastInterruptTime = millis();
    }
}

inline void initButton(uint8_t pin, ButtonActionCallback callback) {
    ButtonHandler::actionCallback = callback;
    pinMode(pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pin), handleInterrupt, FALLING);
}

inline void handleButton() {
    if (ButtonHandler::buttonPressed) {
        ButtonHandler::buttonPressed = false;
        if (ButtonHandler::actionCallback != nullptr) {
            ButtonHandler::actionCallback();
        }
    }
}

#endif // BUTTON_HANDLER_H