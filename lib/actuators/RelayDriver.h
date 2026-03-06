#ifndef RELAY_DRIVER_H
#define RELAY_DRIVER_H

#include <stdint.h>
#include "../hal/IRelayDriver.h"

class RelayDriver : public IRelayDriver {
public:
    RelayDriver() = default;
    virtual ~RelayDriver() = default;

    void begin();

    // IRelayDriver implementation
    void setMainPump(bool on) override;
    void setValve(uint8_t valveId, bool on) override;
};

#endif // RELAY_DRIVER_H
