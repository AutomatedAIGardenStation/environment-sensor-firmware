// Mock Arduino.h for Unity native testing
#ifndef TEST_TANK_EVENTS_ARDUINO_H
#define TEST_TANK_EVENTS_ARDUINO_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifndef NATIVE_TEST_MOCK_SERIAL
#define NATIVE_TEST_MOCK_SERIAL

class MockSerialTank {
public:
    void begin(int baud) {}
    int available() { return 0; }
    int read() { return -1; }
    void print(const char* s);
    void println(const char* s);
    operator bool() const { return true; }
};

extern MockSerialTank Serial;

#endif // NATIVE_TEST_MOCK_SERIAL

uint32_t millis();

// ESP32 Mock
void ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits);
void ledcAttachPin(uint8_t pin, uint8_t channel);
void ledcWrite(uint8_t channel, uint32_t duty);

// AVR Mock
void analogWrite(uint8_t pin, int val);

int analogRead(uint8_t pin);

// GPIO Mock
#define OUTPUT 1
#define INPUT 0
#define HIGH 1
#define LOW 0

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);

// Used in main tests
class String {
public:
    String(float f, int dec = 1) {
        snprintf(buf, sizeof(buf), "%.1f", f);
    }
    const char* c_str() const { return buf; }
private:
    char buf[16];
};

// Global state to inspect in tests
extern char last_emitted_event[256];
extern char all_emitted_events[1024];
extern uint8_t mock_ledc_channel;
extern uint32_t mock_ledc_duty;
extern uint8_t mock_analog_pin;
extern int mock_analog_val;

#endif // ARDUINO_H
