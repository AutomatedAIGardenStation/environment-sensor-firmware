#include "HydraulicWatchdog.h"
#include "../../config/Config.h"
#include "../../src/protocol.h"

HydraulicWatchdog::HydraulicWatchdog(IRelayDriver* driver)
    : driver(driver), active(false), start_ms(0), zone(0) {}

void HydraulicWatchdog::start(uint8_t zone, uint32_t now_ms) {
    this->active = true;
    this->start_ms = now_ms;
    this->zone = zone;
}

void HydraulicWatchdog::stop() {
    this->active = false;
}

void HydraulicWatchdog::tick(uint32_t now_ms) {
    if (active && (now_ms - start_ms) >= MAX_PUMP_TIME_MS) {
        if (driver) {
            driver->setRelay(zone, false);
        }
        protocol_emit_event(EVT_PUMP_OVERCURRENT ":amps=0");
        active = false;
    }
}
