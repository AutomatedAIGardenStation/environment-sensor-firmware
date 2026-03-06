#include "RelayDriver.h"
#include "../../config/Config.h"

#ifdef ARDUINO
#include <Arduino.h>
#else
// For native tests if not ARDUINO
#include "Arduino.h"
#endif

void RelayDriver::begin() {
    pinMode(PIN_MAIN_PUMP, OUTPUT);

    const uint8_t valve_pins[VALVE_COUNT] = { PIN_VALVE_NUT_A, PIN_VALVE_NUT_B, PIN_VALVE_PH_UP, PIN_VALVE_PH_DOWN, PIN_VALVE_CO2 };
    for (uint8_t i = 0; i < VALVE_COUNT; i++) {
        pinMode(valve_pins[i], OUTPUT);
    }

    // Turn them all off initially
    setMainPump(false);
    for (uint8_t i = 0; i < VALVE_COUNT; i++) {
        setValve(i, false);
    }
}

void RelayDriver::setMainPump(bool on) {
#if defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_UNO)
    // Relay outputs (active LOW – relay module pulled to GND by default)
    digitalWrite(PIN_MAIN_PUMP, on ? LOW : HIGH);
#else
    // ESP32 and others (active HIGH)
    digitalWrite(PIN_MAIN_PUMP, on ? HIGH : LOW);
#endif
}

void RelayDriver::setValve(uint8_t valveId, bool on) {
    if (valveId >= VALVE_COUNT) return;

    const uint8_t valve_pins[VALVE_COUNT] = { PIN_VALVE_NUT_A, PIN_VALVE_NUT_B, PIN_VALVE_PH_UP, PIN_VALVE_PH_DOWN, PIN_VALVE_CO2 };
    uint8_t pin = valve_pins[valveId];

#if defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_UNO)
    digitalWrite(pin, on ? LOW : HIGH);
#else
    digitalWrite(pin, on ? HIGH : LOW);
#endif
}
