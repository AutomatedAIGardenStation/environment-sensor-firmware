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
#define VALVE_COUNT               5     // number of independently controllable valves
#define LED_CHANNEL_COUNT         4     // number of LED PWM channels
#define PWM_FREQ                  1000  // 1000 Hz PWM frequency
#define PWM_RESOLUTION            13    // 13-bit PWM resolution

#if defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_UNO)
  // ── Arduino Nano / Uno (ATmega328P) ────────────────────────────────────
  // Relay outputs (active LOW – relay module pulled to GND by default)
  #define PIN_MAIN_PUMP    2
  #define PIN_VALVE_NUT_A  3
  #define PIN_VALVE_NUT_B  4
  #define PIN_VALVE_PH_UP  5
  #define PIN_VALVE_PH_DOWN 6
  #define PIN_VALVE_CO2    7

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


  // Digital / I2C sensors
  #define PIN_I2C_SDA      A4
  #define PIN_I2C_SCL      A5
  // Temperature + humidity: DHT22
  #define PIN_DHT22        0

#elif defined(ARDUINO_ESP32_DEV) || defined(ENV_CONTROLLER) || defined(ESP32)
  // ── ESP32 DevKit ────────────────────────────────────────────────────────
  #define PIN_MAIN_PUMP     4
  #define PIN_VALVE_NUT_A   5
  #define PIN_VALVE_NUT_B   12
  #define PIN_VALVE_PH_UP   13
  #define PIN_VALVE_PH_DOWN 14
  #define PIN_VALVE_CO2     15

  #define PIN_LED_CH1       19
  #define PIN_LED_CH2       21
  #define PIN_LED_CH3       22
  #define PIN_LED_CH4       23

  #define PIN_FAN           25

  #define PIN_PH_SENSOR     34   // Input-only ADC
  #define PIN_EC_SENSOR     35   // Input-only ADC
  #define PIN_TANK_LEVEL    36   // Input-only ADC


  #define PIN_I2C_SDA       27
  #define PIN_I2C_SCL       0

  // ── Wi-Fi & MQTT (ESP32 only) ───────────────────────────────────────────
  #define WIFI_SSID         "GARDEN_WIFI"
  #define WIFI_PASS         "secret123"
  #define MQTT_SERVER       "192.168.1.100"
  #define MQTT_PORT         1883
  #define MQTT_TOPIC_TELEMETRY "garden/env/telemetry"

  #define PIN_DHT22         2

#else
  #warning "Unknown board – check config/Config.h and define your pin mapping."
#endif

// Common overcurrent threshold (adjust to your relay/MOSFET rating)
#define OVERCURRENT_THRESHOLD_MA  4000   // 4 A

#endif  // CONFIG_H
