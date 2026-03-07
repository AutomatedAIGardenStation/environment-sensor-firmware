#ifndef I2C_SENSOR_ADAPTERS_H
#define I2C_SENSOR_ADAPTERS_H

#include <stdint.h>
#include <math.h>
#include "../../config/Config.h"
#include "../util/AveragingBuffer.h"

struct SensorReading {
    float value;
    bool valid;
    uint32_t timestamp;
};

enum class I2CState {
    IDLE,
    WAITING,
    ERROR
};

class AtlasSensorAdapter {
public:
    AtlasSensorAdapter(uint8_t address, uint32_t conv_delay_ms, uint32_t poll_interval_ms, uint32_t stale_timeout_ms, float min_val, float max_val);

    void tick(uint32_t now);
    float getAverage();

    // For testing
    void mockInjectValidRead(float val, uint32_t now) {
        buffer.push(val);
        last_valid_read_ms = now;
    }

private:
    uint8_t addr;
    uint32_t delay_ms;
    uint32_t interval_ms;
    uint32_t stale_ms;
    float min_valid;
    float max_valid;

    I2CState state;
    uint32_t state_entered_ms;
    uint32_t last_poll_ms;
    uint32_t last_valid_read_ms;

    AveragingBuffer<float, 8> buffer;

    void startMeasurement(uint32_t now);
    void readMeasurement(uint32_t now);
};

class SHT31Adapter {
public:
    SHT31Adapter(uint8_t address, uint32_t conv_delay_ms, uint32_t poll_interval_ms, uint32_t stale_timeout_ms);

    void tick(uint32_t now);
    float getTemperatureAverage();
    float getHumidityAverage();

    // For testing
    void mockInjectValidRead(float temp, float hum, uint32_t now) {
        tempBuffer.push(temp);
        humBuffer.push(hum);
        last_valid_read_ms = now;
    }

private:
    uint8_t addr;
    uint32_t delay_ms;
    uint32_t interval_ms;
    uint32_t stale_ms;

    I2CState state;
    uint32_t state_entered_ms;
    uint32_t last_poll_ms;
    uint32_t last_valid_read_ms;

    AveragingBuffer<float, 8> tempBuffer;
    AveragingBuffer<float, 8> humBuffer;

    void startMeasurement(uint32_t now);
    void readMeasurement(uint32_t now);
};

#endif // I2C_SENSOR_ADAPTERS_H
