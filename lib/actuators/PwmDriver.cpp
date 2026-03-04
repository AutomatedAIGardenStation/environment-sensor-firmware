#include "PwmDriver.h"
#include "../../config/Config.h"

#ifdef ARDUINO
#include <Arduino.h>
#elif defined(ESP32) // Allow mocking ESP32 functions in native test
#include "Arduino.h"
#endif

// Mapping logic
static uint32_t map_pct_to_duty(uint8_t pct) {
    if (pct > 100) pct = 100;
    // Duty cycle mapping for the defined resolution
    uint32_t max_duty = (1 << PWM_RESOLUTION) - 1;
    return (uint32_t)pct * max_duty / 100;
}

void PwmDriver::begin() {
#if defined(ARDUINO_ESP32_DEV) || defined(ESP32)
#ifdef PIN_LED_CH1
    // Initialize LED channels 0-3
    const uint8_t led_pins[LED_CHANNEL_COUNT] = { PIN_LED_CH1, PIN_LED_CH2, PIN_LED_CH3, PIN_LED_CH4 };
    for (uint8_t i = 0; i < LED_CHANNEL_COUNT; i++) {
        ledcSetup(i, PWM_FREQ, PWM_RESOLUTION);
        ledcAttachPin(led_pins[i], i);
    }
    // Initialize Fan on channel 4
    ledcSetup(4, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PIN_FAN, 4);
#endif
#endif
}

void PwmDriver::setChannel(uint8_t ch, uint8_t pct) {
    setLedChannel(ch, pct);
}

void PwmDriver::setLedChannel(uint8_t ch, uint8_t pct) {
#ifdef PIN_LED_CH1
    if (ch >= LED_CHANNEL_COUNT) return;
    if (pct > 100) pct = 100;

#if defined(ARDUINO_ESP32_DEV) || defined(ESP32)
    ledcWrite(ch, map_pct_to_duty(pct));
#elif defined(ARDUINO)
    // Fallback for AVR, 8-bit PWM (0-255)
    const uint8_t led_pins[LED_CHANNEL_COUNT] = { PIN_LED_CH1, PIN_LED_CH2, PIN_LED_CH3, PIN_LED_CH4 };
    analogWrite(led_pins[ch], pct * 255 / 100);
#endif
#endif
}

void PwmDriver::setFan(uint8_t pct) {
#ifdef PIN_FAN
    if (pct > 100) pct = 100;

#if defined(ARDUINO_ESP32_DEV) || defined(ESP32)
    ledcWrite(4, map_pct_to_duty(pct));
#elif defined(ARDUINO)
    analogWrite(PIN_FAN, pct * 255 / 100);
#endif
#endif
}
