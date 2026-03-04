#ifndef MOCKSENSORBUS_H
#define MOCKSENSORBUS_H

#include "ISensorBus.h"
#include "../../config/Config.h"

/**
 * Mock implementation of ISensorBus for testing.
 */
class MockSensorBus : public ISensorBus {
public:
    MockSensorBus() : mockTemp(-999.0f), mockHum(-999.0f) {
        for (uint8_t i = 0; i < ZONE_COUNT; ++i) {
            mockSoil[i] = -999.0f;
        }
    }

    void begin() override {
        // Mock hardware initialization
    }

    float readTemperature() override {
        return mockTemp;
    }

    float readHumidity() override {
        return mockHum;
    }

    float readSoilMoisture(uint8_t zone) override {
        if (zone >= ZONE_COUNT) {
            return -999.0f;
        }
        return mockSoil[zone];
    }

    // Setters for mocked data
    void setTemperature(float temp) {
        mockTemp = temp;
    }

    void setHumidity(float hum) {
        mockHum = hum;
    }

    void setSoilMoisture(uint8_t zone, float moisture) {
        if (zone < ZONE_COUNT) {
            mockSoil[zone] = moisture;
        }
    }

private:
    float mockTemp;
    float mockHum;
    float mockSoil[ZONE_COUNT];
};

#endif // MOCKSENSORBUS_H
