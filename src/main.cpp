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
#include "../lib/sensors/SensorBus.h"
#include "../lib/events/EventGuard.h"
#include "../lib/scheduler/PollingEngine.h"
#include "../config/Config.h"
#include <string.h>

#define SERIAL_BAUD      115200
#define LINE_BUF_SIZE    128
#define HEARTBEAT_MS     1000UL    // heartbeat to backend every 1 s
#define SENSOR_POLL_MS   5000UL    // push standard sensor data every 5 s
#define TANK_POLL_MS     1000UL    // check critical tank level every 1 s

static char     g_line_buf[LINE_BUF_SIZE];
static uint8_t  g_line_len       = 0;

PwmDriver g_pwmDriver;
RelayDriver g_relayDriver;
HydraulicWatchdog g_watchdog(&g_relayDriver);
static SensorBus g_sensorBus;
static TankEmptyGuard g_tankEmptyGuard;
static PollingEngine g_pollingEngine;

void task_heartbeat() {
    float t = g_sensorBus.readTemperature();
    float h = g_sensorBus.readHumidity();
    float tank = g_sensorBus.readTankLevel();

    float soil_sum = 0.0f;
    int valid_soil_count = 0;
    for (int i = 0; i < ZONE_COUNT; i++) {
        float s = g_sensorBus.readSoilMoisture(i);
        if (s != -999.0f) {
            soil_sum += s;
            valid_soil_count++;
        }
    }
    float soil_avg = valid_soil_count > 0 ? (soil_sum / valid_soil_count) : -999.0f;

    char t_str[10], h_str[10], soil_str[10], tank_str[10];

    if (t == -999.0f) strcpy(t_str, "-"); else snprintf(t_str, sizeof(t_str), "%.1f", t);
    if (h == -999.0f) strcpy(h_str, "-"); else snprintf(h_str, sizeof(h_str), "%.1f", h);
    if (soil_avg == -999.0f) strcpy(soil_str, "-"); else snprintf(soil_str, sizeof(soil_str), "%.1f", soil_avg);
    if (tank == -999.0f) strcpy(tank_str, "-"); else snprintf(tank_str, sizeof(tank_str), "%.1f", tank);

    char hb_buf[128];
    snprintf(hb_buf, sizeof(hb_buf), "EVT:HEARTBEAT:status=OK:T=%s:H=%s:soil=%s:tank=%s", t_str, h_str, soil_str, tank_str);

    protocol_emit_event(hb_buf);
}

void task_sensors() {
    // TODO: replace with real read – see protocol.cpp read_and_emit_sensors()
    protocol_handle_line("SENSOR_READ");
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
            for (uint8_t i = 0; i < ZONE_COUNT; i++) {
                g_relayDriver.setPump(i, false);
            }
            g_watchdog.stop();
        }
    }
}

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

    g_sensorBus.begin();

    protocol_emit_event("EVT:BOOT:fw=env_controller:v=0.1.0");
}

void loop() {
    // ── Read Serial ──────────────────────────────────────────────────────────
    while (Serial.available()) {
        char c = (char)Serial.read();
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

    uint32_t now = millis();

    g_watchdog.tick(now);
    g_pollingEngine.tick(now);

    // TODO: check overcurrent threshold and emit EVT_PUMP_OVERCURRENT
}
