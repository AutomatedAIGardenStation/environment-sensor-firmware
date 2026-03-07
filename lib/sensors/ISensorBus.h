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
     * Reads Electrical Conductivity (EC).
     * Applies internal averaging to reduce noise.
     * @return EC in mS/cm, or -999.0f on read failure.
     */
    virtual float readEC() = 0;

    /**
     * Reads pH level.
     * Applies internal averaging to reduce noise.
     * @return pH, or -999.0f on read failure.
     */
    virtual float readPH() = 0;

    /**
     * Reads analog tank level value and converts to 0-100%.
     * Applies internal averaging to reduce noise.
     * @return Tank level in %, or -999.0f on read failure.
     */
    virtual float readTankLevel() = 0;

    /**
     * Reads CO2 level from I2C sensor.
     * @return CO2 level in ppm, or -999.0f on read failure.
     */
    virtual float readCO2() = 0;

    /**
     * Ticks the sensor bus non-blocking state machines.
     * @param now Current time in ms.
     */
    virtual void tick(uint32_t now) = 0;
};

#endif // ISENSORBUS_H
