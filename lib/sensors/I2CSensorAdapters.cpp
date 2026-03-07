#include "I2CSensorAdapters.h"
#include <Arduino.h>
#include <stdlib.h>
#ifndef NATIVE_TEST
#include <Wire.h>
#endif

AtlasSensorAdapter::AtlasSensorAdapter(uint8_t address, uint32_t conv_delay_ms, uint32_t poll_interval_ms, uint32_t stale_timeout_ms, float min_val, float max_val)
    : addr(address), delay_ms(conv_delay_ms), interval_ms(poll_interval_ms), stale_ms(stale_timeout_ms),
      min_valid(min_val), max_valid(max_val), state(I2CState::IDLE), state_entered_ms(0),
      last_poll_ms(0), last_valid_read_ms(0) {
}

void AtlasSensorAdapter::tick(uint32_t now) {
    if (state == I2CState::IDLE) {
        if (now - last_poll_ms >= interval_ms) {
            startMeasurement(now);
        }
    } else if (state == I2CState::WAITING) {
        if (now - state_entered_ms >= delay_ms) {
            readMeasurement(now);
        }
    }
}

float AtlasSensorAdapter::getAverage() {
    uint32_t now = millis();
    // Handle the case where time hasn't advanced but we injected a valid read at 0
    if (last_valid_read_ms == 0 && buffer.average() == 0.0f) {
        return -999.0f;
    }

    if (now - last_valid_read_ms > stale_ms) {
        return -999.0f;
    }

    return buffer.average();
}

void AtlasSensorAdapter::startMeasurement(uint32_t now) {
    last_poll_ms = now;
#ifndef NATIVE_TEST
    Wire.beginTransmission(addr);
    Wire.write('R'); // typical read command for EZO
    if (Wire.endTransmission() != 0) {
        // error on bus
        state = I2CState::IDLE;
        return;
    }
#endif

    state = I2CState::WAITING;
    state_entered_ms = now;
}

void AtlasSensorAdapter::readMeasurement(uint32_t now) {
    state = I2CState::IDLE;
#ifndef NATIVE_TEST
    uint8_t bytes = Wire.requestFrom(addr, (uint8_t)32);
    if (bytes == 0) {
        return;
    }

    uint8_t code = Wire.read();
    if (code != 1) { // 1 is successful read for EZO
        // drain buffer
        while (Wire.available()) { Wire.read(); }
        return;
    }

    char read_buf[32];
    uint8_t i = 0;
    while (Wire.available() && i < 31) {
        char c = Wire.read();
        if (c == 0) break;
        read_buf[i++] = c;
    }
    read_buf[i] = '\0';

    float val = atof(read_buf);

    if (!isnan(val) && val >= min_valid && val <= max_valid) {
        buffer.push(val);
        last_valid_read_ms = now;
    }
#else
    // Native test mock path
    // Let tests control or assume valid path. For simplicity, mock can inject direct into buffer.
#endif
}

SHT31Adapter::SHT31Adapter(uint8_t address, uint32_t conv_delay_ms, uint32_t poll_interval_ms, uint32_t stale_timeout_ms)
    : addr(address), delay_ms(conv_delay_ms), interval_ms(poll_interval_ms), stale_ms(stale_timeout_ms),
      state(I2CState::IDLE), state_entered_ms(0), last_poll_ms(0), last_valid_read_ms(0) {
}

void SHT31Adapter::tick(uint32_t now) {
    if (state == I2CState::IDLE) {
        if (now - last_poll_ms >= interval_ms) {
            startMeasurement(now);
        }
    } else if (state == I2CState::WAITING) {
        if (now - state_entered_ms >= delay_ms) {
            readMeasurement(now);
        }
    }
}

float SHT31Adapter::getTemperatureAverage() {
    uint32_t now = millis();
    if (last_valid_read_ms == 0 && tempBuffer.average() == 0.0f) {
        return -999.0f;
    }
    if (now - last_valid_read_ms > stale_ms) {
        return -999.0f;
    }
    return tempBuffer.average();
}

float SHT31Adapter::getHumidityAverage() {
    uint32_t now = millis();
    if (last_valid_read_ms == 0 && humBuffer.average() == 0.0f) {
        return -999.0f;
    }
    if (now - last_valid_read_ms > stale_ms) {
        return -999.0f;
    }
    return humBuffer.average();
}

void SHT31Adapter::startMeasurement(uint32_t now) {
    last_poll_ms = now;
#ifndef NATIVE_TEST
    Wire.beginTransmission(addr);
    Wire.write(0x24); // SHT31 high repeatability measurement
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        state = I2CState::IDLE;
        return;
    }
#endif
    state = I2CState::WAITING;
    state_entered_ms = now;
}

void SHT31Adapter::readMeasurement(uint32_t now) {
    state = I2CState::IDLE;
#ifndef NATIVE_TEST
    uint8_t bytes = Wire.requestFrom(addr, (uint8_t)6);
    if (bytes != 6) {
        return;
    }

    uint16_t t_raw = (Wire.read() << 8) | Wire.read();
    Wire.read(); // CRC
    uint16_t h_raw = (Wire.read() << 8) | Wire.read();
    Wire.read(); // CRC

    float t = -45.0f + 175.0f * ((float)t_raw / 65535.0f);
    float h = 100.0f * ((float)h_raw / 65535.0f);

    if (!isnan(t) && !isnan(h) && t >= -40.0f && t <= 125.0f && h >= 0.0f && h <= 100.0f) {
        tempBuffer.push(t);
        humBuffer.push(h);
        last_valid_read_ms = now;
    }
#else
    // Mock path
#endif
}
