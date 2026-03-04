/**
 * env_controller – board-specific pin assignments.
 *
 * Update this file for your chosen MCU and wiring.
 * Supported targets: Arduino Nano/Uno (ATmega328P), ESP32.
 */
#ifndef PINS_H
#define PINS_H

#if defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_UNO)
  // ── Arduino Nano / Uno (ATmega328P) ────────────────────────────────────
  // Relay outputs (active LOW – relay module pulled to GND by default)
  #define PIN_PUMP_ZONE1   2    // Water pump zone 1
  #define PIN_PUMP_ZONE2   3    // Water pump zone 2
  #define PIN_VALVE_ZONE1  4    // Solenoid valve zone 1
  #define PIN_VALVE_ZONE2  5    // Solenoid valve zone 2
  #define PIN_NUTRIENT_PUMP 6   // Nutrient dosing pump
  // PWM outputs
  #define PIN_LED_CH1      9    // Grow LED channel 1 (PWM)
  #define PIN_LED_CH2      10   // Grow LED channel 2 (PWM)
  #define PIN_FAN          11   // Fan speed control (PWM)
  // Analog sensor inputs
  #define PIN_PH_SENSOR    A0
  #define PIN_EC_SENSOR    A1
  #define PIN_TANK_LEVEL   A2
  // Digital / I2C sensors
  // Temperature + humidity: DHT22 on pin 7, or BME280 over I2C (A4/A5)
  #define PIN_DHT22        7

#elif defined(ARDUINO_ESP32_DEV)
  // ── ESP32 DevKit ────────────────────────────────────────────────────────
  #define PIN_PUMP_ZONE1    4
  #define PIN_PUMP_ZONE2    5
  #define PIN_VALVE_ZONE1   12
  #define PIN_VALVE_ZONE2   13
  #define PIN_NUTRIENT_PUMP 14
  #define PIN_LED_CH1       25   // DAC / LEDC PWM capable
  #define PIN_LED_CH2       26
  #define PIN_FAN           27
  #define PIN_PH_SENSOR     34   // Input-only ADC
  #define PIN_EC_SENSOR     35   // Input-only ADC
  #define PIN_TANK_LEVEL    36   // Input-only ADC
  #define PIN_DHT22         15

#else
  #warning "Unknown board – check config/pins.h and define your pin mapping."
#endif

// Common overcurrent threshold (adjust to your relay/MOSFET rating)
#define OVERCURRENT_THRESHOLD_MA  4000   // 4 A

#endif  // PINS_H
