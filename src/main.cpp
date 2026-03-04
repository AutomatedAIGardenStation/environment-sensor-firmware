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

#define SERIAL_BAUD      115200
#define LINE_BUF_SIZE    128
#define HEARTBEAT_MS     5000UL    // heartbeat to backend every 5 s
#define SENSOR_POLL_MS   10000UL   // push sensor data every 10 s

static char     g_line_buf[LINE_BUF_SIZE];
static uint8_t  g_line_len       = 0;
static uint32_t g_last_hb_ms     = 0;
static uint32_t g_last_sensor_ms = 0;

static PwmDriver g_pwmDriver;
static RelayDriver g_relayDriver;
static HydraulicWatchdog g_watchdog(&g_relayDriver);

void setup() {
    Serial.begin(SERIAL_BAUD);
    while (!Serial) { /* wait for USB-serial on native-USB boards */ }
    g_line_buf[0]   = '\0';
    g_line_len      = 0;
    g_last_hb_ms    = millis();
    g_last_sensor_ms = millis();

    // Initialize PWM Driver
    g_pwmDriver.begin();
    protocol_set_pwm_driver(&g_pwmDriver);

    // Initialize Relay Driver
    g_relayDriver.begin();
    protocol_set_relay_driver(&g_relayDriver);
    protocol_set_watchdog(&g_watchdog);

    // TODO: initialise sensor buses here
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

    // ── Heartbeat ────────────────────────────────────────────────────────────
    if ((now - g_last_hb_ms) >= HEARTBEAT_MS) {
        protocol_emit_event(EVT_HEARTBEAT ":status=OK");
        g_last_hb_ms = now;
    }

    // ── Periodic sensor push ─────────────────────────────────────────────────
    if ((now - g_last_sensor_ms) >= SENSOR_POLL_MS) {
        // TODO: replace with real read – see protocol.cpp read_and_emit_sensors()
        protocol_handle_line("SENSOR_READ");
        g_last_sensor_ms = now;
    }

    // TODO: check overcurrent / tank-low thresholds and emit relevant EVT_* events
}
