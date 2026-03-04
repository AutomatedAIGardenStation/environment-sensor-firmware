#ifndef IPWM_DRIVER_H
#define IPWM_DRIVER_H

#include <stdint.h>

class IPwmDriver {
public:
    virtual ~IPwmDriver() = default;
    virtual void setChannel(uint8_t ch, uint8_t pct) = 0;
};

#endif // IPWM_DRIVER_H
