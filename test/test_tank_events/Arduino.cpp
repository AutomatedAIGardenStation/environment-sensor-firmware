#include "Arduino.h"
#include <string.h>

MockSerialTank Serial;

// Global buffer for last event printed to Serial
char last_emitted_event[256];
char all_emitted_events[1024];

void MockSerialTank::print(const char* s) {
    // optional stub
}

void MockSerialTank::println(const char* s) {
    strncpy(last_emitted_event, s, sizeof(last_emitted_event) - 1);
    last_emitted_event[sizeof(last_emitted_event) - 1] = '\0';

    // Also append to all_emitted_events
    strncat(all_emitted_events, s, sizeof(all_emitted_events) - strlen(all_emitted_events) - 1);
    strncat(all_emitted_events, "\n", sizeof(all_emitted_events) - strlen(all_emitted_events) - 1);
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

int analogRead(uint8_t pin); // Implemented in test_tank_events.cpp

void pinMode(uint8_t pin, uint8_t mode) {}
void digitalWrite(uint8_t pin, uint8_t val); // Implemented in test_tank_events.cpp
