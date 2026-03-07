/**
 * env_controller – command protocol.
 * Implements the ASCII line protocol defined in:
 *   project-info/Docs/06_Software/env_controller/Serial Protocol & Interfaces.md
 *
 * All commands are issued by the backend's Serial Manager.
 * Baud: 115200  Frame: 1 start, 8 data, 1 stop, no parity.
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

// ── Inbound commands (backend → env_controller) ──────────────────────────────
#define CMD_PUMP_RUN     "PUMP_RUN"      // :ms=<time>
#define CMD_PUMP_MAIN    "PUMP_MAIN"     // :state=<0|1>
#define CMD_WATER_STOP   "WATER_STOP"
#define CMD_VALVE_SET    "VALVE_SET"     // :id=<name>:state=<0|1>
#define CMD_DOSE_RECIPE  "DOSE_RECIPE"   // :NutA=<ms>:NutB=<ms>:pH_Up=<ms>:pH_Down=<ms>
#define CMD_DOSE_STOP    "DOSE_STOP"

#define CMD_LIGHT_MODE   "LIGHT_MODE"    // :<preset>
#define CMD_FAN_SET      "FAN_SET"       // :pct=<0-100>
#define CMD_HEAT_SET     "HEAT_SET"      // :pct=<0-100>
#define CMD_NOP          "NOP"

// ── Outbound events (env_controller → backend) ──────────────────────────────
#define EVT_WATER_DONE       "EVT:WATER_DONE"
#define EVT_TANK_LOW         "EVT:TANK_LOW"         // :level_pct=<n>
#define EVT_TANK_EMPTY       "EVT:TANK_EMPTY"       // :level_pct=<n>
#define EVT_PUMP_OVERCURRENT "EVT:PUMP_OVERCURRENT" // :amps=<n>
#define EVT_TEMP_HIGH        "EVT:TEMP_HIGH"        // :deg_c=<n>
#define EVT_HEARTBEAT        "EVT:HEARTBEAT"        // :status=OK

class PwmDriver; // Forward declaration
class IRelayDriver;
class HydraulicWatchdog;
class Doser;

// ── Public API ───────────────────────────────────────────────────────────────
bool protocol_handle_line(const char* line);

/**
 * Emits an event.
 * On ESP32, this publishes to MQTT (if connected).
 * On AVR/fallback, it prints to Serial.
 */
void protocol_emit_event(const char* event);

/**
 * Perform any network connection setup/loop tasks.
 * Should be called from main.cpp's loop() and setup().
 */
void protocol_net_loop();
void protocol_net_begin();

// Setter to allow dependency injection for the PwmDriver
void protocol_set_pwm_driver(PwmDriver* driver);

// Setters for new drivers
void protocol_set_relay_driver(IRelayDriver* driver);
void protocol_set_watchdog(HydraulicWatchdog* wd);
void protocol_set_doser(Doser* doser);

#endif  // PROTOCOL_H
