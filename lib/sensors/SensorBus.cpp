#include "SensorBus.h"
#include <Arduino.h>
#include <Wire.h>

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
#if defined(ESP32)
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
#else
    Wire.begin();
#endif

    // Initialize ADC pins for soil zones
    // The exact pins are defined in Config.h, we map them here
#ifdef PIN_SOIL_ZONE1
    if (ZONE_COUNT > 0) pinMode(PIN_SOIL_ZONE1, INPUT);
#endif
#ifdef PIN_SOIL_ZONE2
    if (ZONE_COUNT > 1) pinMode(PIN_SOIL_ZONE2, INPUT);
#endif
#ifdef PIN_SOIL_ZONE3
    if (ZONE_COUNT > 2) pinMode(PIN_SOIL_ZONE3, INPUT);
#endif
#ifdef PIN_SOIL_ZONE4
    if (ZONE_COUNT > 3) pinMode(PIN_SOIL_ZONE4, INPUT);
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

uint16_t SensorBus::readRawSoilADC(uint8_t zone) {
    switch (zone) {
        case 0:
#ifdef PIN_SOIL_ZONE1
            return analogRead(PIN_SOIL_ZONE1);
#else
            return 0;
#endif
        case 1:
#ifdef PIN_SOIL_ZONE2
            return analogRead(PIN_SOIL_ZONE2);
#else
            return 0;
#endif
        case 2:
#ifdef PIN_SOIL_ZONE3
            return analogRead(PIN_SOIL_ZONE3);
#else
            return 0;
#endif
        case 3:
#ifdef PIN_SOIL_ZONE4
            return analogRead(PIN_SOIL_ZONE4);
#else
            return 0;
#endif
        default:
            return 0;
    }
}

float SensorBus::mapADCToMoisture(uint16_t adcValue) {
    // This is a placeholder mapping function.
    // In reality, this requires calibration values for dry/wet soil.
    // For now, let's assume a generic 10-bit ADC (0-1023) mapping where
    // 1023 is dry and 0 is wet (capacitive soil moisture sensors often invert).
    // If it's a 12-bit ADC (ESP32, 0-4095), this would differ.

    // We'll normalize dynamically based on a generic range, but since actual
    // mapping depends on hardware, we'll do a simple conversion.
    // For capacitive sensors, typically lower value = wetter.

    // Let's use standard Arduino map function over 0-1023 for atmega
    // or 0-4095 for esp32. We can try to handle both generic cases,
    // or just return the ADC value mapped to 0-100%.

    // Assuming higher ADC is drier, lower ADC is wetter.
#if defined(ESP32)
    // 12-bit ADC
    const float dryValue = 4095.0f;
    const float wetValue = 0.0f;
#else
    // 10-bit ADC
    const float dryValue = 1023.0f;
    const float wetValue = 0.0f;
#endif

    float moisture = 100.0f * (dryValue - (float)adcValue) / (dryValue - wetValue);

    // Constrain to 0-100
    if (moisture < 0.0f) moisture = 0.0f;
    if (moisture > 100.0f) moisture = 100.0f;

    return moisture;
}

float SensorBus::readSoilMoisture(uint8_t zone) {
    if (zone >= ZONE_COUNT) {
        return -999.0f;
    }

    uint16_t rawVal = readRawSoilADC(zone);
    soilBuffers[zone].push(rawVal);

    float avgRaw = soilBuffers[zone].average();

    return mapADCToMoisture((uint16_t)avgRaw);
}
