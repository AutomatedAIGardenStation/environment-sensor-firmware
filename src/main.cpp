/**
 * env_controller – main entry point.
 *
 * Reads ASCII command lines from Serial (backend Serial Manager), one per line.
 * Baud: 115200.
 *
 * Architecture:
 *   Backend ──serial──► env_controller (this firmware) ──HAL──► Pumps / LEDs / Fans / Sensors
 *   env_controller ──serial──► Backend  (events: EVT:SENSOR_DATA, EVT:WATER_DONE, …)
 *
 * See: project-info/Docs/06_Software/env_controller/Serial Protocol & Interfaces.md
 */
#include <Arduino.h>
#include "protocol.h"
#include "../lib/actuators/PwmDriver.h"
#include "../lib/actuators/RelayDriver.h"
#include "../lib/actuators/HydraulicWatchdog.h"
#include "../lib/actuators/Doser.h"
#include "../lib/sensors/SensorBus.h"
#include "../lib/events/EventGuard.h"
#include "../lib/scheduler/PollingEngine.h"
#include "../config/Config.h"
#include <string.h>

#define SERIAL_BAUD      115200
#define LINE_BUF_SIZE    128
#define HEARTBEAT_MS     1000UL    // heartbeat to backend every 1 s
#define TANK_POLL_MS     1000UL    // check critical tank level every 1 s
#define SENSOR_POLL_MS   30000UL   // 30 seconds for sensor data

static char     g_line_buf[LINE_BUF_SIZE];
static uint8_t  g_line_len       = 0;

static uint32_t g_last_rx_ms     = 0;

PwmDriver g_pwmDriver;
RelayDriver g_relayDriver;
HydraulicWatchdog g_watchdog(&g_relayDriver);
Doser g_doser(&g_relayDriver);

static SensorBus g_sensorBus;
static TankEmptyGuard g_tankEmptyGuard;
static PollingEngine g_pollingEngine;

void task_heartbeat() {
    char hb_buf[64];
    snprintf(hb_buf, sizeof(hb_buf), "EVT:HEARTBEAT:status=OK");
    protocol_emit_event(hb_buf);
}

void task_sensors() {
    float t = g_sensorBus.readTemperature();
    float h = g_sensorBus.readHumidity();
    float ec = g_sensorBus.readEC();
    float ph = g_sensorBus.readPH();

    char buf[128];

    if (t != -999.0f) {
        snprintf(buf, sizeof(buf), "EVT:AIR_TEMP:temp=%.1f", t);
        protocol_emit_event(buf);
    }
    if (h != -999.0f) {
        snprintf(buf, sizeof(buf), "EVT:AIR_HUMIDITY:humidity=%.1f", h);
        protocol_emit_event(buf);
    }
    if (ec != -999.0f || ph != -999.0f || t != -999.0f) {
        // We use water_temp=0 for now or same as air_temp depending on design.
        // Let's use air temp for now, or just leave it empty.
        snprintf(buf, sizeof(buf), "EVT:SENSOR_UPDATE:ec=%.2f:ph=%.2f:water_temp=%.1f",
                 (ec != -999.0f ? ec : 0.0), (ph != -999.0f ? ph : 0.0), (t != -999.0f ? t : 0.0));
        protocol_emit_event(buf);

        // Check thresholds and emit
        if (ec != -999.0f) {
            if (ec < 1.0f) protocol_emit_event("EVT:EC_LOW"); // Placeholder thresholds
            else if (ec > 3.0f) protocol_emit_event("EVT:EC_HIGH");
        }
        if (ph != -999.0f) {
            if (ph < 5.5f) protocol_emit_event("EVT:PH_LOW");
            else if (ph > 6.5f) protocol_emit_event("EVT:PH_HIGH");
        }
    }

    // Add missing CO2 telemetry separately to avoid breaking EVT:SENSOR_UPDATE format
    float co2 = g_sensorBus.readCO2();
    if (co2 != -999.0f) {
        snprintf(buf, sizeof(buf), "EVT:CO2_LEVEL:co2=%.0f", co2);
        protocol_emit_event(buf);

        if (co2 < 400.0f) {
            protocol_emit_event("EVT:CO2_LOW");
        }
    }
}

void task_tank_level() {
    float tankLevel = g_sensorBus.readTankLevel();
    if (tankLevel != -999.0f) {
        bool isBelowThreshold = (tankLevel < TANK_EMPTY_THRESHOLD_PCT);
        if (g_tankEmptyGuard.check(isBelowThreshold)) {
            // Emit EVT:TANK_EMPTY once per transition
            char evt_buf[64];
            snprintf(evt_buf, sizeof(evt_buf), "%s:level_pct=%d", EVT_TANK_EMPTY, (int)tankLevel);
            protocol_emit_event(evt_buf);

            // Immediate pump stop
            g_relayDriver.setMainPump(false);
            g_watchdog.stop();
        }
    }
}

#if defined(ESP32) && !defined(NATIVE_TEST)
void core0_task(void *pvParameters) {
    while (true) {
        uint32_t now = millis();
        // Currently the polling engine tasks log data directly,
        // which now goes into the queue.
        g_pollingEngine.tick(now);
        protocol_net_loop();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
#endif

void setup() {
    Serial.begin(SERIAL_BAUD);
    while (!Serial) { /* wait for USB-serial on native-USB boards */ }
    g_line_buf[0]   = '\0';
    g_line_len      = 0;

    // Reset polling engine by clearing all tasks
    g_pollingEngine = PollingEngine();

    g_pollingEngine.addTask("tank_level", TANK_POLL_MS, task_tank_level);
    g_pollingEngine.addTask("sensors", SENSOR_POLL_MS, task_sensors);
    g_pollingEngine.addTask("heartbeat", HEARTBEAT_MS, task_heartbeat);

    // Initialize PWM Driver
    g_pwmDriver.begin();
    protocol_set_pwm_driver(&g_pwmDriver);

    // Initialize Relay Driver
    g_relayDriver.begin();
    protocol_set_relay_driver(&g_relayDriver);
    protocol_set_watchdog(&g_watchdog);
    protocol_set_doser(&g_doser);

    g_sensorBus.begin();

    protocol_emit_event("EVT:BOOT:fw=env_controller:v=0.1.0");
    protocol_net_begin();

    g_last_rx_ms = millis();

#if defined(ESP32) && !defined(NATIVE_TEST)
    xTaskCreatePinnedToCore(
        core0_task,     // Function to implement the task
        "core0_task",   // Name of the task
        4096,           // Stack size in words
        NULL,           // Task input parameter
        1,              // Priority of the task
        NULL,           // Task handle
        0               // Core where the task should run
    );
#endif
}

void loop() {
    uint32_t now = millis();

    // ── Read Serial ──────────────────────────────────────────────────────────
    // cppcheck-suppress knownConditionTrueFalse
    while (Serial.available()) {
        char c = static_cast<char>(Serial.read());
        g_last_rx_ms = now; // Update PING watchdog
        // cppcheck-suppress knownConditionTrueFalse
        if (c == '\n' || c == '\r') {
            if (g_line_len > 0) {
                g_line_buf[g_line_len] = '\0';
                protocol_handle_line(g_line_buf);
                g_line_len = 0;
            }
        } else if (g_line_len < (LINE_BUF_SIZE - 1)) {
            g_line_buf[g_line_len++] = c;
        }
        // Silently drop bytes when buffer is full
    }

    // PING Watchdog (10s silence)
    if (now - g_last_rx_ms > 10000UL) {
        g_relayDriver.setMainPump(false);
        for (uint8_t i = 0; i < VALVE_COUNT; i++) {
            g_relayDriver.setValve(i, false);
        }
        g_watchdog.stop();
        g_doser.stop();
        g_last_rx_ms = now; // Prevent repeating
        protocol_emit_event("EVT:PING_TIMEOUT");
    }

    g_watchdog.tick(now);
    g_doser.tick(now);

#if defined(ESP32) && !defined(NATIVE_TEST)
    // On ESP32, polling engine and network loop run on Core 0
#else
    // On AVR/native, they run here in the main loop
    g_pollingEngine.tick(now);
    protocol_net_loop();
#endif

    g_sensorBus.tick(now);
}
