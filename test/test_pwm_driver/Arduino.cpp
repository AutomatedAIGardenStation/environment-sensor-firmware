#include "Arduino.h"
#include <string.h>

#ifndef NATIVE_TEST_MOCK_SERIAL_INSTANTIATED
#define NATIVE_TEST_MOCK_SERIAL_INSTANTIATED
MockSerial Serial;
#endif

// Global buffer for last event printed to Serial
char last_emitted_event[256];

void MockSerial::print(const char* s) {
    // optional stub
}

void MockSerial::println(const char* s) {
    strncpy(last_emitted_event, s, sizeof(last_emitted_event) - 1);
    last_emitted_event[sizeof(last_emitted_event) - 1] = '\0';
}

uint8_t mock_ledc_channel = 0xFF;
uint32_t mock_ledc_duty = 0xFFFFFFFF;

uint8_t mock_analog_pin = 0xFF;
int mock_analog_val = -1;

void ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits) {}
void ledcAttachPin(uint8_t pin, uint8_t channel) {}

void ledcWrite(uint8_t channel, uint32_t duty) {
    mock_ledc_channel = channel;
    mock_ledc_duty = duty;
}

void analogWrite(uint8_t pin, int val) {
    mock_analog_pin = pin;
    mock_analog_val = val;
}

int analogRead(uint8_t pin) {
    return mock_analog_val;
}

void pinMode(uint8_t pin, uint8_t mode) {}
void digitalWrite(uint8_t pin, uint8_t val) {}
