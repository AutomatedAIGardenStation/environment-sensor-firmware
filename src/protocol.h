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
#define CMD_WATER_START  "WATER_START"   // :zone=<id>
#define CMD_WATER_STOP   "WATER_STOP"
#define CMD_FEED         "F1"            // Nutrient dose trigger
#define CMD_LIGHT_SET    "LIGHT_SET"     // :ch=<n>:pct=<0-100>
#define CMD_FAN_SET      "FAN_SET"       // :pct=<0-100>
#define CMD_SENSOR_READ  "SENSOR_READ"   // Request immediate sensor snapshot
#define CMD_NOP          "NOP"

// ── Outbound events (env_controller → backend) ──────────────────────────────
#define EVT_WATER_DONE       "EVT:WATER_DONE"
#define EVT_TANK_LOW         "EVT:TANK_LOW"         // :level_pct=<n>
#define EVT_TANK_EMPTY       "EVT:TANK_EMPTY"       // :level_pct=<n>
#define EVT_PUMP_OVERCURRENT "EVT:PUMP_OVERCURRENT"  // :amps=<n>
#define EVT_TEMP_HIGH        "EVT:TEMP_HIGH"         // :deg_c=<n>
#define EVT_SENSOR_DATA      "EVT:SENSOR_DATA"       // :temp=<n>:hum=<n>:ph=<n>:ec=<n>:level=<n>
#define EVT_HEARTBEAT        "EVT:HEARTBEAT"         // :status=OK

class PwmDriver; // Forward declaration
class IRelayDriver;
class HydraulicWatchdog;

// ── Public API ───────────────────────────────────────────────────────────────
bool protocol_handle_line(const char* line);
void protocol_emit_event(const char* event);

// Setter to allow dependency injection for the PwmDriver
void protocol_set_pwm_driver(PwmDriver* driver);

// Setters for new drivers
void protocol_set_relay_driver(IRelayDriver* driver);
void protocol_set_watchdog(HydraulicWatchdog* wd);

#endif  // PROTOCOL_H
