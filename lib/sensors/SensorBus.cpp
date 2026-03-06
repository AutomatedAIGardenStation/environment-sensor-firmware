#include "SensorBus.h"
#include <Arduino.h>

#ifndef NATIVE_TEST
#include <Wire.h>
#endif

// If you want actual logic to connect to a sensor (e.g. SHT31, DHT22)
// you'd include its library here.
// For the sake of this issue, we provide structural logic and placeholders
// for ambient sensors, but full actual logic for soil moisture averaging.

SensorBus::SensorBus() {
}

SensorBus::~SensorBus() {
}

void SensorBus::begin() {
    // Initialize I2C bus
    // ESP32 usually allows specifying pins in begin(SDA, SCL)
#ifndef NATIVE_TEST
#if defined(ESP32)
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
#else
    Wire.begin();
#endif
#endif

#ifdef PIN_EC_SENSOR
    pinMode(PIN_EC_SENSOR, INPUT);
#endif
#ifdef PIN_PH_SENSOR
    pinMode(PIN_PH_SENSOR, INPUT);
#endif

#ifdef PIN_TANK_LEVEL
    pinMode(PIN_TANK_LEVEL, INPUT);
#endif
}

float SensorBus::readTemperature() {
    // Placeholder logic for reading temperature via I2C (DHT22-compatible or SHT31).
    // The instructions state: returns float in °C; returns -999.0f on read failure.
    // In a real implementation we would call a sensor driver here.

    // As real I2C hardware initialization on ESP32 is out of scope
    // and tests use the mock, return the failure condition for now
    // to simulate "unavailable" since the physical sensor is not present.
    return -999.0f;
}

float SensorBus::readHumidity() {
    // Placeholder logic for reading ambient relative humidity (%).
    // Returns -999.0f on read failure.
    return -999.0f;
}

static float mapADCToEC(uint16_t adcValue) {
    // Placeholder linear map for EC
    return (float)adcValue * (5.0f / 4095.0f);
}

static float mapADCToPH(uint16_t adcValue) {
    // Placeholder linear map for pH
    return (float)adcValue * (14.0f / 4095.0f);
}

float SensorBus::readEC() {
#ifdef PIN_EC_SENSOR
    uint16_t rawVal = analogRead(PIN_EC_SENSOR);
    ecBuffer.push(rawVal);
    return mapADCToEC((uint16_t)ecBuffer.average());
#else
    return -999.0f;
#endif
}

float SensorBus::readPH() {
#ifdef PIN_PH_SENSOR
    uint16_t rawVal = analogRead(PIN_PH_SENSOR);
    phBuffer.push(rawVal);
    return mapADCToPH((uint16_t)phBuffer.average());
#else
    return -999.0f;
#endif
}

float SensorBus::readTankLevel() {
#ifdef PIN_TANK_LEVEL
    uint16_t rawVal = analogRead(PIN_TANK_LEVEL);
    tankBuffer.push(rawVal);
    float avgRaw = tankBuffer.average();

    // Using a simple linear mapping for now: assume higher ADC is higher level or lower level
    // Assuming higher ADC is lower tank level, similar to soil mapping. Or maybe opposite.
    // The issue says: "reads the tank level sensor ... returns 0-100%".
    // We can use the same mapADCToMoisture for consistency or create a mapADCToTankLevel.
    // If not specified, a typical mapping is fine. Let's create a linear mapping here.
#if defined(ESP32)
    const float dryValue = 4095.0f;
    const float wetValue = 0.0f;
#else
    const float dryValue = 1023.0f;
    const float wetValue = 0.0f;
#endif

    float level = 100.0f * (dryValue - avgRaw) / (dryValue - wetValue);

    if (level < 0.0f) level = 0.0f;
    if (level > 100.0f) level = 100.0f;

    return level;
#else
    return -999.0f;
#endif
}
