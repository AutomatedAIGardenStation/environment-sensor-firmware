/**
 * env_controller – board-specific pin assignments and configuration.
 *
 * Update this file for your chosen MCU and wiring.
 * Supported targets: Arduino Nano/Uno (ATmega328P), ESP32.
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// ── Global Configuration ──────────────────────────────────────────────────
#define MAX_PUMP_TIME_MS          30000 // 30 seconds, hard watchdog limit
#define TANK_EMPTY_THRESHOLD_PCT  10    // percent below which EVT:TANK_EMPTY fires
#define ZONE_COUNT                4     // number of independently controllable water zones
#define LED_CHANNEL_COUNT         4     // number of LED PWM channels

#if defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_UNO)
  // ── Arduino Nano / Uno (ATmega328P) ────────────────────────────────────
  // Relay outputs (active LOW – relay module pulled to GND by default)
  #define PIN_PUMP_ZONE1   2
  #define PIN_PUMP_ZONE2   3
  #define PIN_PUMP_ZONE3   4
  #define PIN_PUMP_ZONE4   5
  #define PIN_VALVE_ZONE1  6
  #define PIN_VALVE_ZONE2  7
  #define PIN_VALVE_ZONE3  8
  #define PIN_VALVE_ZONE4  12
  #define PIN_NUTRIENT_PUMP 13

  // PWM outputs
  #define PIN_LED_CH1      9
  #define PIN_LED_CH2      10
  #define PIN_LED_CH3      11
  #define PIN_LED_CH4      A3
  #define PIN_FAN          A2

  // Analog sensor inputs
  #define PIN_PH_SENSOR    A0
  #define PIN_EC_SENSOR    A1
  #define PIN_TANK_LEVEL   A4

  // Soil Moisture ADCs
  #define PIN_SOIL_ZONE1   A6
  #define PIN_SOIL_ZONE2   A7
  #define PIN_SOIL_ZONE3   A5
  #define PIN_SOIL_ZONE4   1

  // Digital / I2C sensors
  #define PIN_I2C_SDA      A4
  #define PIN_I2C_SCL      A5
  // Temperature + humidity: DHT22
  #define PIN_DHT22        0

#elif defined(ARDUINO_ESP32_DEV) || defined(ENV_CONTROLLER) || defined(ESP32)
  // ── ESP32 DevKit ────────────────────────────────────────────────────────
  #define PIN_PUMP_ZONE1    4
  #define PIN_PUMP_ZONE2    5
  #define PIN_PUMP_ZONE3    12
  #define PIN_PUMP_ZONE4    13

  #define PIN_VALVE_ZONE1   14
  #define PIN_VALVE_ZONE2   15
  #define PIN_VALVE_ZONE3   16
  #define PIN_VALVE_ZONE4   17

  #define PIN_NUTRIENT_PUMP 18

  #define PIN_LED_CH1       19
  #define PIN_LED_CH2       21
  #define PIN_LED_CH3       22
  #define PIN_LED_CH4       23

  #define PIN_FAN           25

  #define PIN_PH_SENSOR     34   // Input-only ADC
  #define PIN_EC_SENSOR     35   // Input-only ADC
  #define PIN_TANK_LEVEL    36   // Input-only ADC

  #define PIN_SOIL_ZONE1    39   // Input-only ADC
  #define PIN_SOIL_ZONE2    32
  #define PIN_SOIL_ZONE3    33
  #define PIN_SOIL_ZONE4    26

  #define PIN_I2C_SDA       27
  #define PIN_I2C_SCL       0

  #define PIN_DHT22         2

#else
  #warning "Unknown board – check config/Config.h and define your pin mapping."
#endif

// Common overcurrent threshold (adjust to your relay/MOSFET rating)
#define OVERCURRENT_THRESHOLD_MA  4000   // 4 A

#endif  // CONFIG_H
