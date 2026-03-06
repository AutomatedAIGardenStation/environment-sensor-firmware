#ifndef MOCKSENSORBUS_H
#define MOCKSENSORBUS_H

#include "ISensorBus.h"
#include "../../config/Config.h"

/**
 * Mock implementation of ISensorBus for testing.
 */
class MockSensorBus : public ISensorBus {
public:
    MockSensorBus() : mockTemp(-999.0f), mockHum(-999.0f), mockEC(-999.0f), mockPH(-999.0f), mockTankLevel(-999.0f) {
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

    float readEC() override {
        return mockEC;
    }

    float readPH() override {
        return mockPH;
    }

    float readTankLevel() override {
        return mockTankLevel;
    }

    // Setters for mocked data
    void setTemperature(float temp) { mockTemp = temp; }
    void setHumidity(float hum) { mockHum = hum; }
    void setEC(float ec) { mockEC = ec; }
    void setPH(float ph) { mockPH = ph; }
    void setTankLevel(float level) { mockTankLevel = level; }

private:
    float mockTemp;
    float mockHum;
    float mockEC;
    float mockPH;
    float mockTankLevel;
};

#endif // MOCKSENSORBUS_H
