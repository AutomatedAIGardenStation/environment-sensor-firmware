#ifndef IRELAY_DRIVER_H
#define IRELAY_DRIVER_H

#include <stdint.h>

class IRelayDriver {
public:
    virtual ~IRelayDriver() = default;
    virtual void setRelay(uint8_t zone, bool on) = 0;
};

#endif // IRELAY_DRIVER_H
