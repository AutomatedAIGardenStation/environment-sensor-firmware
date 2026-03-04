/**
 * env_controller – Serial protocol implementation.
 * Dispatches inbound ASCII lines from the backend and emits sensor/status events.
 *
 * TODO: Replace stub actions with real peripheral calls for your hardware.
 */
#include "protocol.h"
#include <Arduino.h>
#include <string.h>   // strncmp, strlen, memchr

// ── helpers ──────────────────────────────────────────────────────────────────

static inline bool cmd_match(const char* line, const char* cmd, size_t line_len) {
    size_t cmd_len = strlen(cmd);
    if (line_len < cmd_len) return false;
    return strncmp(line, cmd, cmd_len) == 0
        && (line_len == cmd_len || line[cmd_len] == ':');
}

// ── stubs ─────────────────────────────────────────────────────────────────────

static void pump_start(const char* params) {
    // TODO: enable relay/MOSFET for the requested zone pump
    // Parse ":zone=<id>" from params if needed
    (void)params;
}

static void pump_stop_all() {
    // TODO: disable all pump relays
}

static void nutrient_dose() {
    // TODO: briefly enable the nutrient pump
}

static void light_set(const char* params) {
    // TODO: parse ":ch=<n>:pct=<0-100>" and write PWM to LED channel
    (void)params;
}

static void fan_set(const char* params) {
    // TODO: parse ":pct=<0-100>" and write PWM to fan output
    (void)params;
}

static void read_and_emit_sensors() {
    // TODO: read all sensors (temp, humidity, pH, EC, tank level) and emit:
    //   EVT:SENSOR_DATA:temp=<n>:hum=<n>:ph=<n>:ec=<n>:level=<n>
    // Example (replace with real sensor reads):
    Serial.println("EVT:SENSOR_DATA:temp=0.0:hum=0.0:ph=0.0:ec=0.0:level=0");
}

// ── public API ───────────────────────────────────────────────────────────────

bool protocol_handle_line(const char* line) {
    if (!line) return false;
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n' || line[len - 1] == ' '))
        len--;
    if (len == 0) return false;

    const char* params = nullptr;
    const char* sep = static_cast<const char*>(memchr(line, ':', len));
    if (sep) params = sep + 1;

    if (cmd_match(line, CMD_WATER_START, len)) {
        pump_start(params ? params : "");
        return true;
    }
    if (cmd_match(line, CMD_WATER_STOP, len)) {
        pump_stop_all();
        protocol_emit_event(EVT_WATER_DONE);
        return true;
    }
    if (cmd_match(line, CMD_FEED, len)) {
        nutrient_dose();
        return true;
    }
    if (cmd_match(line, CMD_LIGHT_SET, len)) {
        light_set(params ? params : "");
        return true;
    }
    if (cmd_match(line, CMD_FAN_SET, len)) {
        fan_set(params ? params : "");
        return true;
    }
    if (cmd_match(line, CMD_SENSOR_READ, len)) {
        read_and_emit_sensors();
        return true;
    }
    if (cmd_match(line, CMD_NOP, len)) {
        return true;
    }

    Serial.print("ERR:UNKNOWN:");
    Serial.println(line);
    return false;
}

void protocol_emit_event(const char* event) {
    Serial.println(event);
}
