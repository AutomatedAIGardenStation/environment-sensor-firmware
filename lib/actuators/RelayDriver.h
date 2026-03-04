#ifndef RELAY_DRIVER_H
#define RELAY_DRIVER_H

#include <stdint.h>
#include "../hal/IRelayDriver.h"

class RelayDriver : public IRelayDriver {
public:
    RelayDriver() = default;
    virtual ~RelayDriver() = default;

    void begin();

    // Specific to RelayDriver
    void setPump(uint8_t zone, bool on);

    // IRelayDriver implementation
    void setRelay(uint8_t zone, bool on) override;
};

#endif // RELAY_DRIVER_H
