#include "SensorBus.h"
#include <Arduino.h>

#ifndef NATIVE_TEST
#include <Wire.h>
#endif

// If you want actual logic to connect to a sensor (e.g. SHT31, DHT22)
// you'd include its library here.
// For the sake of this issue, we provide structural logic and placeholders
// for ambient sensors, but full actual logic for soil moisture averaging.

SensorBus::SensorBus()
    : phAdapter(I2C_ADDR_PH, SENSOR_CONV_DELAY_MS, SENSOR_POLL_INTERVAL_MS, SENSOR_STALE_TIMEOUT_MS, 0.0f, 14.0f),
      ecAdapter(I2C_ADDR_EC, SENSOR_CONV_DELAY_MS, SENSOR_POLL_INTERVAL_MS, SENSOR_STALE_TIMEOUT_MS, 0.0f, 10.0f),
      co2Adapter(I2C_ADDR_CO2, SENSOR_CONV_DELAY_MS, SENSOR_POLL_INTERVAL_MS, SENSOR_STALE_TIMEOUT_MS, 0.0f, 5000.0f),
      tempHumAdapter(I2C_ADDR_SHT31, SHT31_CONV_DELAY_MS, SENSOR_POLL_INTERVAL_MS, SENSOR_STALE_TIMEOUT_MS),
      last_tank_poll_ms(0) {
}

SensorBus::~SensorBus() {
}

void SensorBus::tick(uint32_t now) {
    phAdapter.tick(now);
    ecAdapter.tick(now);
    co2Adapter.tick(now);
    tempHumAdapter.tick(now);

    // Poll tank level every interval
    if (now - last_tank_poll_ms >= SENSOR_POLL_INTERVAL_MS) {
        last_tank_poll_ms = now;
#ifdef PIN_TANK_LEVEL
        uint16_t rawVal = analogRead(PIN_TANK_LEVEL);
        tankBuffer.push(rawVal);
#endif
    }
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
    return tempHumAdapter.getTemperatureAverage();
}

float SensorBus::readHumidity() {
    return tempHumAdapter.getHumidityAverage();
}

float SensorBus::readEC() {
    return ecAdapter.getAverage();
}

float SensorBus::readPH() {
    return phAdapter.getAverage();
}

float SensorBus::readCO2() {
    return co2Adapter.getAverage();
}

float SensorBus::readTankLevel() {
#ifdef PIN_TANK_LEVEL
    // No valid reads pushed yet
    if (tankBuffer.average() == 0.0f && tankBuffer.isFull() == false) {
        return -999.0f;
    }

    float avgRaw = tankBuffer.average();

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
