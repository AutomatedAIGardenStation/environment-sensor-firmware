#ifndef HYDRAULIC_WATCHDOG_H
#define HYDRAULIC_WATCHDOG_H

#include <stdint.h>
#include "../hal/IRelayDriver.h"

class HydraulicWatchdog {
public:
    HydraulicWatchdog(IRelayDriver* driver);

    void start(uint8_t zone, uint32_t now_ms);
    void stop();
    void tick(uint32_t now_ms);

private:
    IRelayDriver* driver;
    bool active;
    uint32_t start_ms;
    uint8_t zone;
};

#endif // HYDRAULIC_WATCHDOG_H
