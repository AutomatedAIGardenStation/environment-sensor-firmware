#ifndef MOCK_PWM_DRIVER_H
#define MOCK_PWM_DRIVER_H

#include "PwmDriver.h"

class MockPwmDriver : public PwmDriver {
public:
    uint8_t last_ch = 0xFF;
    uint8_t last_pct = 0xFF;
    uint8_t last_fan_pct = 0xFF;

    MockPwmDriver() = default;
    virtual ~MockPwmDriver() = default;

    void setLedChannel(uint8_t ch, uint8_t pct) override {
        last_ch = ch;
        // Do not clamp in the mock so we can test what is passed to it
        last_pct = pct;
    }

    void setFan(uint8_t pct) override {
        last_fan_pct = pct;
    }
};

#endif // MOCK_PWM_DRIVER_H
