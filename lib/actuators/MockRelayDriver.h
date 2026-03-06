#ifndef MOCK_RELAY_DRIVER_H
#define MOCK_RELAY_DRIVER_H

#include <stdint.h>
#include "../hal/IRelayDriver.h"
#include "../../config/Config.h"

class MockRelayDriver : public IRelayDriver {
public:
    bool main_pump_state;
    bool valve_states[VALVE_COUNT];

    MockRelayDriver() {
        main_pump_state = false;
        for (uint8_t i = 0; i < VALVE_COUNT; i++) {
            valve_states[i] = false;
        }
    }

    void setMainPump(bool on) override {
        main_pump_state = on;
    }

    void setValve(uint8_t valveId, bool on) override {
        if (valveId < VALVE_COUNT) {
            valve_states[valveId] = on;
        }
    }
};

#endif // MOCK_RELAY_DRIVER_H
