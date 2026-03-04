#ifndef ISENSOR_BUS_H
#define ISENSOR_BUS_H

#include <stdint.h>

class ISensorBus {
public:
    virtual ~ISensorBus() = default;
    virtual float readTemperature() = 0;
    virtual float readHumidity() = 0;
    virtual float readSoilMoisture(uint8_t zone) = 0;
    virtual float readTankLevel() = 0;
};

#endif // ISENSOR_BUS_H
