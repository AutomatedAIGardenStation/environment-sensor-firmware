/**
 * env_controller – Serial protocol implementation.
 * Dispatches inbound ASCII lines from the backend and emits sensor/status events.
 *
 * TODO: Replace stub actions with real peripheral calls for your hardware.
 */
#include "protocol.h"
#include <Arduino.h>
#include <stdlib.h>
#include <string.h>   // strncmp, strlen, memchr
#include "../lib/actuators/PwmDriver.h"
#include "../lib/actuators/HydraulicWatchdog.h"
#include "../lib/actuators/Doser.h"
#include "../lib/hal/IRelayDriver.h"
#include "../config/Config.h"

// Wi-Fi and MQTT logic for ESP32
#if defined(ESP32) && !defined(NATIVE_TEST)
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
// Since PubSubClient.h might be missing in some CI runs or library scopes
// without modifying global libdeps, we conditionally declare it.
#if __has_include(<PubSubClient.h>)
#include <PubSubClient.h>
static WiFiClient espClient;
static PubSubClient mqttClient(espClient);
#define HAS_PUBSUB
#endif

#define EVENT_QUEUE_LENGTH 20
#define EVENT_QUEUE_ITEM_SIZE 128
static QueueHandle_t g_eventQueue = nullptr;

#endif

// Global driver instance set by main or tests
PwmDriver* protocol_g_pwmDriver = nullptr;
IRelayDriver* protocol_g_relayDriver = nullptr;
HydraulicWatchdog* protocol_g_watchdog = nullptr;
Doser* protocol_g_doser = nullptr;

// ── helpers ──────────────────────────────────────────────────────────────────

static uint8_t crc8(const uint8_t* data, size_t len) {
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

static inline bool cmd_match(const char* line, const char* cmd, size_t line_len) {
    size_t cmd_len = strlen(cmd);
    if (line_len < cmd_len) return false;
    return strncmp(line, cmd, cmd_len) == 0
        && (line_len == cmd_len || line[cmd_len] == ':');
}

// ── implementation ────────────────────────────────────────────────────────────

static void pump_run(const char* params) {
    uint32_t ms = MAX_PUMP_TIME_MS;
    if (params) {
        const char* ms_str = strstr(params, "ms=");
        if (ms_str) {
            ms = atoi(ms_str + 3);
        }
    }

    if (protocol_g_relayDriver) {
        protocol_g_relayDriver->setMainPump(true);
    }
    if (protocol_g_watchdog) {
        protocol_g_watchdog->start(millis(), ms);
    }
}

static void pump_main(const char* params) {
    if (!params) return;
    const char* state_str = strstr(params, "state=");
    if (state_str) {
        bool state = atoi(state_str + 6) != 0;
        if (protocol_g_relayDriver) {
            protocol_g_relayDriver->setMainPump(state);
        }
        if (state && protocol_g_watchdog) {
            protocol_g_watchdog->start(millis());
        } else if (!state && protocol_g_watchdog) {
            protocol_g_watchdog->stop();
        }
    }
}

static void valve_set(const char* params) {
    if (!params) return;
    const char* id_str = strstr(params, "id=");
    const char* state_str = strstr(params, "state=");
    if (id_str && state_str) {
        uint8_t id = 255;
        if (strncmp(id_str + 3, "NutA", 4) == 0) id = 0;
        else if (strncmp(id_str + 3, "NutB", 4) == 0) id = 1;
        else if (strncmp(id_str + 3, "pH_Up", 5) == 0) id = 2;
        else if (strncmp(id_str + 3, "pH_Down", 7) == 0) id = 3;
        else if (strncmp(id_str + 3, "CO2", 3) == 0) id = 4;

        bool state = atoi(state_str + 6) != 0;
        if (id < VALVE_COUNT && protocol_g_relayDriver) {
            protocol_g_relayDriver->setValve(id, state);
        }
    }
}

static void dose_recipe(const char* params) {
    if (!params || !protocol_g_doser) return;
    uint32_t msA = 0, msB = 0, msPhUp = 0, msPhDown = 0;

    const char* pA = strstr(params, "NutA=");
    if (pA) msA = atoi(pA + 5);

    const char* pB = strstr(params, "NutB=");
    if (pB) msB = atoi(pB + 5);

    const char* pUp = strstr(params, "pH_Up=");
    if (pUp) msPhUp = atoi(pUp + 6);

    const char* pDown = strstr(params, "pH_Down=");
    if (pDown) msPhDown = atoi(pDown + 8);

    protocol_g_doser->startDose(msA, msB, msPhUp, msPhDown, millis());
}

static void dose_stop() {
    if (protocol_g_doser) {
        protocol_g_doser->stop();
    }
}

static void pump_stop_all() {
    if (protocol_g_relayDriver) {
        protocol_g_relayDriver->setMainPump(false);
        for (uint8_t i = 0; i < VALVE_COUNT; i++) {
            protocol_g_relayDriver->setValve(i, false);
        }
    }
    if (protocol_g_watchdog) {
        protocol_g_watchdog->stop();
    }
    if (protocol_g_doser) {
        protocol_g_doser->stop();
    }
}

static void light_set(const char* params) {
    if (!protocol_g_pwmDriver || !params) return;

    // Parse ":ch=<n>:pct=<0-100>"
    const char* ch_str = strstr(params, "ch=");
    const char* pct_str = strstr(params, "pct=");

    if (ch_str && pct_str) {
        uint8_t ch = atoi(ch_str + 3);
        uint8_t pct = atoi(pct_str + 4);
        protocol_g_pwmDriver->setLedChannel(ch, pct);
    }
}

static void fan_set(const char* params) {
    if (!protocol_g_pwmDriver || !params) return;

    // Parse ":pct=<0-100>"
    const char* pct_str = strstr(params, "pct=");
    if (pct_str) {
        uint8_t pct = atoi(pct_str + 4);
        protocol_g_pwmDriver->setFan(pct);
    }
}

// Stubs for now
static void heat_set(const char* params) {}

void protocol_set_pwm_driver(PwmDriver* driver) {
    protocol_g_pwmDriver = driver;
}

void protocol_set_relay_driver(IRelayDriver* driver) {
    protocol_g_relayDriver = driver;
}

void protocol_set_watchdog(HydraulicWatchdog* wd) {
    protocol_g_watchdog = wd;
}

void protocol_set_doser(Doser* doser) {
    protocol_g_doser = doser;
}

// ── public API ───────────────────────────────────────────────────────────────

bool protocol_handle_line(const char* line) {
    if (!line) return false;
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n' || line[len - 1] == ' '))
        len--;
    if (len == 0) return false;

    char buf[128];
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    memcpy(buf, line, len);
    buf[len] = '\0';

    char* first_colon = strchr(buf, ':');
    if (!first_colon) {
        Serial.print("ERR:UNKNOWN:");
        Serial.println(buf);
        return false;
    }
    *first_colon = '\0';
    const char* seq_str = buf;

    char* crc_ptr = strstr(first_colon + 1, ":CRC=");
    if (!crc_ptr) {
        Serial.print("NACK:");
        Serial.print(seq_str);
        Serial.println(":BADCRC");
        return false;
    }

    // include the colon before CRC=
    size_t payload_len = (crc_ptr - buf) + 1;
    uint8_t calculated_crc = crc8(reinterpret_cast<const uint8_t*>(line), payload_len);

    const char* crc_hex_str = crc_ptr + 5;
    char hex_buf[3] = {0};
    strncpy(hex_buf, crc_hex_str, 2);
    uint8_t received_crc = (uint8_t)strtol(hex_buf, nullptr, 16);

    if (calculated_crc != received_crc) {
        Serial.print("NACK:");
        Serial.print(seq_str);
        Serial.println(":BADCRC");
        return false;
    }

    *crc_ptr = '\0';
    const char* cmd_str = first_colon + 1;
    size_t cmd_len = strlen(cmd_str);

    const char* params = nullptr;
    const char* sep = strchr(cmd_str, ':');
    if (sep) params = sep + 1;

    bool handled = false;

    if (cmd_match(cmd_str, CMD_PUMP_RUN, cmd_len)) {
        pump_run(params ? params : "");
        handled = true;
    } else if (cmd_match(cmd_str, CMD_PUMP_MAIN, cmd_len)) {
        pump_main(params ? params : "");
        handled = true;
    } else if (cmd_match(cmd_str, CMD_WATER_STOP, cmd_len)) {
        pump_stop_all();
        protocol_emit_event(EVT_WATER_DONE);
        handled = true;
    } else if (cmd_match(cmd_str, CMD_VALVE_SET, cmd_len)) {
        valve_set(params ? params : "");
        handled = true;
    } else if (cmd_match(cmd_str, CMD_DOSE_RECIPE, cmd_len)) {
        dose_recipe(params ? params : "");
        handled = true;
    } else if (cmd_match(cmd_str, CMD_DOSE_STOP, cmd_len)) {
        dose_stop();
        handled = true;
    } else if (cmd_match(cmd_str, CMD_LIGHT_SET, cmd_len)) {
        light_set(params ? params : "");
        handled = true;
    } else if (cmd_match(cmd_str, CMD_FAN_SET, cmd_len)) {
        fan_set(params ? params : "");
        handled = true;
    } else if (cmd_match(cmd_str, CMD_HEAT_SET, cmd_len)) {
        heat_set(params ? params : "");
        handled = true;
    } else if (cmd_match(cmd_str, CMD_NOP, cmd_len)) {
        handled = true;
    }

    if (handled) {
        Serial.print("ACK:");
        Serial.println(seq_str);
        return true;
    } else {
        Serial.print("NACK:");
        Serial.print(seq_str);
        Serial.println(":UNKNOWN");
        return false;
    }
}

void protocol_emit_event(const char* event) {
#if defined(ESP32) && !defined(NATIVE_TEST)
    if (g_eventQueue) {
        char buf[EVENT_QUEUE_ITEM_SIZE];
        strncpy(buf, event, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        if (xQueueSend(g_eventQueue, buf, 0) != pdTRUE) {
            Serial.println("ERR:QUEUE_FULL");
        }
    } else {
        Serial.println(event);
    }
#else
    Serial.println(event);
#endif
}

void protocol_net_begin() {
#if defined(ESP32) && !defined(NATIVE_TEST)
    g_eventQueue = xQueueCreate(EVENT_QUEUE_LENGTH, EVENT_QUEUE_ITEM_SIZE);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
#if defined(HAS_PUBSUB)
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
#endif
#endif
}

void protocol_net_loop() {
#if defined(ESP32) && !defined(NATIVE_TEST)
    bool isConnected = false;

    if (WiFi.status() == WL_CONNECTED) {
#if defined(HAS_PUBSUB)
        if (!mqttClient.connected()) {
            if (mqttClient.connect("EnvControllerClient")) {
                // connected
            }
        }
        if (mqttClient.connected()) {
            mqttClient.loop();
            isConnected = true;
        }
#endif
    }

    if (g_eventQueue) {
        char buf[EVENT_QUEUE_ITEM_SIZE];
        while (xQueueReceive(g_eventQueue, buf, 0) == pdTRUE) {
#if defined(HAS_PUBSUB)
            if (isConnected) {
                mqttClient.publish(MQTT_TOPIC_TELEMETRY, buf);
            } else {
                Serial.println(buf); // Fallback
            }
#else
            (void)isConnected; // Prevent unused variable warning
            Serial.println(buf); // Fallback
#endif
        }
    }
#endif
}
