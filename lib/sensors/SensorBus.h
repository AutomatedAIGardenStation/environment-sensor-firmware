#ifndef SENSORBUS_H
#define SENSORBUS_H

#include "ISensorBus.h"
#include "../../config/Config.h"
#include "../util/AveragingBuffer.h"

/**
 * Concrete implementation of ISensorBus for reading physical sensors.
 */
class SensorBus : public ISensorBus {
public:
    SensorBus();
    ~SensorBus() override;

    void begin() override;
    float readTemperature() override;
    float readHumidity() override;
    float readSoilMoisture(uint8_t zone) override;

private:
    // Averaging buffers for each soil zone
    AveragingBuffer<uint16_t, 8> soilBuffers[ZONE_COUNT];

    // Read raw analog ADC value for the specific zone
    uint16_t readRawSoilADC(uint8_t zone);

    // Platform-dependent function to map ADC value to 0-100% moisture
    float mapADCToMoisture(uint16_t adcValue);
};

#endif // SENSORBUS_H
