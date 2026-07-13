#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

// Оголошуємо тип для функції зворотного виклику (callback)
typedef void (*ButtonActionCallback)();

// Глобальні змінні для модуля
namespace ButtonHandler {
    extern uint8_t buttonPin;
    extern ButtonActionCallback actionCallback;

    extern int lastButtonState;
    extern int buttonState;
    extern unsigned long lastDebounceTime;
    const unsigned long debounceDelay = 50;
}

inline void initButton(uint8_t pin, ButtonActionCallback callback) {
    ButtonHandler::buttonPin = pin;
    ButtonHandler::actionCallback = callback;
    pinMode(ButtonHandler::buttonPin, INPUT_PULLUP);
}

inline void handleButton() {
    int reading = digitalRead(ButtonHandler::buttonPin);

    if (reading != ButtonHandler::lastButtonState) {
        ButtonHandler::lastDebounceTime = millis();
    }

    if ((millis() - ButtonHandler::lastDebounceTime) > ButtonHandler::debounceDelay && reading != ButtonHandler::buttonState) {
        ButtonHandler::buttonState = reading;
        if (ButtonHandler::buttonState == LOW && ButtonHandler::actionCallback != nullptr) {
            ButtonHandler::actionCallback(); // Викликаємо передану функцію
        }
    }
    ButtonHandler::lastButtonState = reading;
}

#endif // BUTTON_HANDLER_H