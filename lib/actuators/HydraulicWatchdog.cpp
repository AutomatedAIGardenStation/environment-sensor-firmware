#include "HydraulicWatchdog.h"
#include "../../config/Config.h"
#include "../../src/protocol.h"

HydraulicWatchdog::HydraulicWatchdog(IRelayDriver* driver)
    : driver(driver), active(false), start_ms(0), timeout_ms(MAX_PUMP_TIME_MS) {}

void HydraulicWatchdog::start(uint32_t now_ms) {
    start(now_ms, MAX_PUMP_TIME_MS);
}

void HydraulicWatchdog::start(uint32_t now_ms, uint32_t duration_ms) {
    this->active = true;
    this->start_ms = now_ms;
    this->timeout_ms = (duration_ms > MAX_PUMP_TIME_MS) ? MAX_PUMP_TIME_MS : duration_ms;
}

void HydraulicWatchdog::stop() {
    this->active = false;
}

void HydraulicWatchdog::tick(uint32_t now_ms) {
    if (active && (now_ms - start_ms) >= timeout_ms) {
        if (driver) {
            driver->setMainPump(false);
        }
        // If it hit the max limit, maybe we say overcurrent or timeout
        if (timeout_ms == MAX_PUMP_TIME_MS) {
            protocol_emit_event(EVT_PUMP_OVERCURRENT ":amps=0");
        }
        active = false;
    }
}
