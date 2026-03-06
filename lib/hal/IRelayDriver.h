#ifndef IRELAY_DRIVER_H
#define IRELAY_DRIVER_H

#include <stdint.h>

class IRelayDriver {
public:
    virtual ~IRelayDriver() = default;
    virtual void setMainPump(bool on) = 0;
    virtual void setValve(uint8_t valveId, bool on) = 0;
};

#endif // IRELAY_DRIVER_H
