#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

// Оголошуємо тип для функції зворотного виклику (callback)
typedef void (*ButtonCallback)();

// Глобальні змінні для модуля
namespace ButtonHandler {
    ButtonCallback singleClickCallback = nullptr;
    ButtonCallback doubleClickCallback = nullptr;
    ButtonCallback longPressCallback = nullptr;

    // Змінні для стану кнопки
    volatile bool buttonState = HIGH; // Поточний стан (HIGH = не натиснуто)
    volatile bool lastButtonState = HIGH; // Попередній стан
    volatile unsigned long lastDebounceTime = 0;

    // Змінні для логіки кліків
    int clickCount = 0;
    unsigned long lastClickTime = 0;
    bool longPressHandled = false;

    // Налаштування часу
    const unsigned long debounceDelay = 50;
    const unsigned long doubleClickDelay = 400;
    const unsigned long longPressDelay = 3000; // 3 секунди
}

inline void initButton(uint8_t pin, ButtonCallback singleClickCb, ButtonCallback doubleClickCb, ButtonCallback longPressCb) {
    ButtonHandler::singleClickCallback = singleClickCb;
    ButtonHandler::doubleClickCallback = doubleClickCb;
    ButtonHandler::longPressCallback = longPressCb;
    pinMode(pin, INPUT_PULLUP);
}

inline void handleButton() {
    bool reading = digitalRead(DISPLAY_BUTTON_PIN);

    // Якщо стан змінився, скидаємо таймер брязкоту
    if (reading != ButtonHandler::lastButtonState) {
        ButtonHandler::lastDebounceTime = millis();
    }

    if ((millis() - ButtonHandler::lastDebounceTime) > ButtonHandler::debounceDelay) {
        // Якщо стан стабілізувався
        if (reading != ButtonHandler::buttonState) {
            ButtonHandler::buttonState = reading;

            if (ButtonHandler::buttonState == LOW) { // Кнопку натиснуто
                ButtonHandler::lastClickTime = millis();
                ButtonHandler::longPressHandled = false;
            } else { // Кнопку відпущено
                if (!ButtonHandler::longPressHandled) {
                    ButtonHandler::clickCount++;
                }
            }
        }
    }

    // Обробка довгого натискання
    if (ButtonHandler::buttonState == LOW && !ButtonHandler::longPressHandled && (millis() - ButtonHandler::lastClickTime > ButtonHandler::longPressDelay)) {
        if (ButtonHandler::longPressCallback) ButtonHandler::longPressCallback();
        ButtonHandler::longPressHandled = true;
    }

    // Обробка одинарних та подвійних кліків
    if (ButtonHandler::clickCount > 0 && (millis() - ButtonHandler::lastClickTime > ButtonHandler::doubleClickDelay)) {
        if (ButtonHandler::clickCount == 1) {
            if (ButtonHandler::singleClickCallback) ButtonHandler::singleClickCallback();
        } else if (ButtonHandler::clickCount >= 2) {
            if (ButtonHandler::doubleClickCallback) ButtonHandler::doubleClickCallback();
        }
        // Скидаємо лічильник після обробки
        ButtonHandler::clickCount = 0;
    }


    ButtonHandler::lastButtonState = reading;
}

#endif // BUTTON_HANDLER_H