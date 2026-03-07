#ifndef MOCKSENSORBUS_H
#define MOCKSENSORBUS_H

#include "ISensorBus.h"
#include "../../config/Config.h"

/**
 * Mock implementation of ISensorBus for testing.
 */
class MockSensorBus : public ISensorBus {
public:
    MockSensorBus() : mockTemp(-999.0f), mockHum(-999.0f), mockEC(-999.0f), mockPH(-999.0f), mockTankLevel(-999.0f), mockCO2(-999.0f) {
    }

    void begin() override {
        // Mock hardware initialization
    }

    void tick(uint32_t now) override {
        // Mock non-blocking tick
    }

    float readTemperature() override {
        return mockTemp;
    }

    float readHumidity() override {
        return mockHum;
    }

    float readEC() override {
        return mockEC;
    }

    float readPH() override {
        return mockPH;
    }

    float readTankLevel() override {
        return mockTankLevel;
    }

    float readCO2() override {
        return mockCO2;
    }

    // Setters for mocked data
    void setTemperature(float temp) { mockTemp = temp; }
    void setHumidity(float hum) { mockHum = hum; }
    void setEC(float ec) { mockEC = ec; }
    void setPH(float ph) { mockPH = ph; }
    void setTankLevel(float level) { mockTankLevel = level; }
    void setCO2(float co2) { mockCO2 = co2; }

private:
    float mockTemp;
    float mockHum;
    float mockEC;
    float mockPH;
    float mockTankLevel;
    float mockCO2;
};

#endif // MOCKSENSORBUS_H
