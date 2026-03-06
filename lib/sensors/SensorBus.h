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
    float readEC() override;
    float readPH() override;
    float readTankLevel() override;

private:
    AveragingBuffer<uint16_t, 8> ecBuffer;
    AveragingBuffer<uint16_t, 8> phBuffer;
    AveragingBuffer<uint16_t, 4> tankBuffer;

    float mapADCToEC(uint16_t adcValue);
    float mapADCToPH(uint16_t adcValue);
};

#endif // SENSORBUS_H
