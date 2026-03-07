#ifndef SENSORBUS_H
#define SENSORBUS_H

#include "ISensorBus.h"
#include "../../config/Config.h"
#include "../util/AveragingBuffer.h"
#include "I2CSensorAdapters.h"

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
    float readCO2() override;

    void tick(uint32_t now) override;

private:
    AtlasSensorAdapter phAdapter;
    AtlasSensorAdapter ecAdapter;
    AtlasSensorAdapter co2Adapter;
    SHT31Adapter tempHumAdapter;

    AveragingBuffer<uint16_t, 4> tankBuffer;
    uint32_t last_tank_poll_ms;
};

#endif // SENSORBUS_H
