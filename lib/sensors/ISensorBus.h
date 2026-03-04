#ifndef ISENSORBUS_H
#define ISENSORBUS_H

#include <stdint.h>

/**
 * Interface for reading environmental data from physical sensors.
 *
 * Sensor read failures should return -999.0f to indicate a "sensor unavailable"
 * condition to the protocol layer.
 */
class ISensorBus {
public:
    virtual ~ISensorBus() {}

    /**
     * Initializes hardware interfaces (e.g., I2C bus, ADC pins).
     */
    virtual void begin() = 0;

    /**
     * Reads ambient temperature.
     * @return Temperature in °C, or -999.0f on read failure.
     */
    virtual float readTemperature() = 0;

    /**
     * Reads ambient relative humidity.
     * @return Humidity in %, or -999.0f on read failure.
     */
    virtual float readHumidity() = 0;

    /**
     * Reads analog soil moisture value for the given zone and converts to 0-100%.
     * Applies internal averaging to reduce noise.
     * @param zone The soil zone index (e.g., 0-3 for 4 zones).
     * @return Soil moisture in %, or -999.0f on invalid zone or read failure.
     */
    virtual float readSoilMoisture(uint8_t zone) = 0;

    /**
     * Reads analog tank level value and converts to 0-100%.
     * Applies internal averaging to reduce noise.
     * @return Tank level in %, or -999.0f on read failure.
     */
    virtual float readTankLevel() = 0;
};

#endif // ISENSORBUS_H
