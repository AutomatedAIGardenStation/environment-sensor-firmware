// Mock Arduino.h for Unity native testing
#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

class MockSerial {
public:
    void begin(int baud) {}
    int available() { return 0; }
    int read() { return -1; }
    void print(const char* s);
    void println(const char* s);
    operator bool() const { return true; }
};

extern MockSerial Serial;

uint32_t millis();

// ESP32 Mock
void ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits);
void ledcAttachPin(uint8_t pin, uint8_t channel);
void ledcWrite(uint8_t channel, uint32_t duty);

// AVR Mock
void analogWrite(uint8_t pin, int val);

// GPIO Mock
#define OUTPUT 1
#define INPUT 0
#define HIGH 1
#define LOW 0

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);

// Global state to inspect in tests
extern char last_emitted_event[256];
extern uint8_t mock_ledc_channel;
extern uint32_t mock_ledc_duty;
extern uint8_t mock_analog_pin;
extern int mock_analog_val;

#endif // ARDUINO_H
