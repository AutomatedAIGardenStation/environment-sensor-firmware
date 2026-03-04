#include "Arduino.h"

MockSerial Serial;

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
