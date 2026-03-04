#ifndef PWM_DRIVER_H
#define PWM_DRIVER_H

#include <stdint.h>
#include "../hal/IPwmDriver.h"

class PwmDriver : public IPwmDriver {
public:
    PwmDriver() = default;
    virtual ~PwmDriver() = default;

    void begin();

    // IPwmDriver implementation
    void setChannel(uint8_t ch, uint8_t pct) override;

    // Specific methods
    virtual void setLedChannel(uint8_t ch, uint8_t pct);
    virtual void setFan(uint8_t pct);
};

#endif // PWM_DRIVER_H
