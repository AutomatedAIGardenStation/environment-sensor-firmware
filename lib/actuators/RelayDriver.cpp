#include "RelayDriver.h"
#include "../../config/Config.h"

#ifdef ARDUINO
#include <Arduino.h>
#else
// For native tests if not ARDUINO
#include "Arduino.h"
#endif

void RelayDriver::begin() {
    const uint8_t pump_pins[ZONE_COUNT] = { PIN_PUMP_ZONE1, PIN_PUMP_ZONE2, PIN_PUMP_ZONE3, PIN_PUMP_ZONE4 };

    for (uint8_t i = 0; i < ZONE_COUNT; i++) {
        pinMode(pump_pins[i], OUTPUT);
    }

    // Turn them all off initially
    for (uint8_t i = 0; i < ZONE_COUNT; i++) {
        setPump(i, false);
    }
}

void RelayDriver::setPump(uint8_t zone, bool on) {
    if (zone >= ZONE_COUNT) return;

    const uint8_t pump_pins[ZONE_COUNT] = { PIN_PUMP_ZONE1, PIN_PUMP_ZONE2, PIN_PUMP_ZONE3, PIN_PUMP_ZONE4 };
    uint8_t pin = pump_pins[zone];

#if defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_UNO)
    // Relay outputs (active LOW – relay module pulled to GND by default)
    digitalWrite(pin, on ? LOW : HIGH);
#else
    // ESP32 and others (active HIGH)
    digitalWrite(pin, on ? HIGH : LOW);
#endif
}

void RelayDriver::setRelay(uint8_t zone, bool on) {
    setPump(zone, on);
}
