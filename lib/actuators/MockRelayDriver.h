#ifndef MOCK_RELAY_DRIVER_H
#define MOCK_RELAY_DRIVER_H

#include <stdint.h>
#include "../hal/IRelayDriver.h"
#include "../../config/Config.h"

class MockRelayDriver : public IRelayDriver {
public:
    bool pump_states[ZONE_COUNT];

    MockRelayDriver() {
        for (uint8_t i = 0; i < ZONE_COUNT; i++) {
            pump_states[i] = false;
        }
    }

    void setRelay(uint8_t zone, bool on) override {
        if (zone < ZONE_COUNT) {
            pump_states[zone] = on;
        }
    }
};

#endif // MOCK_RELAY_DRIVER_H
