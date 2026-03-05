# env_controller

Firmware for the **environmental subsystem** of Garden Station.

Handles: water pumps, solenoid valves, nutrient dosing, LED grow lights, fans / heaters, and all environmental sensors (temperature, humidity, pH, EC, tank level).

> **Architecture**: The backend's Serial Manager is the only process that sends commands to this controller. The firmware never communicates with other software modules directly.
>
> Full protocol spec: `project-info/Docs/06_Software/env_controller/Serial Protocol & Interfaces.md`

---

## Quick Start

```bash
# Build (requires PlatformIO CLI)
pio run -e nanoatmega328

# Flash to target
pio run -e nanoatmega328 -t upload

# Monitor serial output
pio device monitor -b 115200

# Run native unit tests
pio test -e native
```

Available environments: `nanoatmega328`, `uno`, `esp32dev`, `native`

---

## Project Structure

```
env_controller/
├── platformio.ini      # PlatformIO environments for supported boards
├── config/
│   └── Config.h        # Board-specific pin assignments and global constants
├── lib/
│   ├── actuators/      # Actuator HAL (PwmDriver, RelayDriver)
│   ├── events/         # Event dispatch and guards
│   ├── scheduler/      # Polling engine and timed tasks
│   └── sensors/        # Sensor bus and HAL
├── src/
│   ├── main.cpp        # Setup / loop, Serial reader, heartbeat, sensor polling
│   ├── protocol.h      # Command & event constants, public API
│   └── protocol.cpp    # Command dispatcher
└── test/               # Unity test suites (run with pio test -e native)
```

---

## Serial Protocol (summary)

| Direction | Format |
|-----------|--------|
| Backend → Controller | `<COMMAND>[:<key>=<val>]*\n` |
| Controller → Backend | `EVT:<NAME>[:<key>=<val>]*\n` |

### Commands

| Command | Parameters | Description |
|---|---|---|
| `WATER_START` | `zone=<id>` (0-3) | Starts a water pump for a specific zone. |
| `WATER_STOP` | (none) | Stops all active pumps immediately. |
| `W1` | (none) | Starts pump for zone 0. (Legacy command) |
| `FEED` or `F1` | (none) | Triggers a quick nutrient dose. |
| `LIGHT_SET` | `ch=<id>` (0-3), `pct=<0-100>` | Sets PWM intensity for an LED channel. |
| `L1` | (none) | Sets all 4 LED channels to 100%. (Legacy command) |
| `FAN_SET` | `pct=<0-100>` | Sets PWM intensity for the fan. |
| `SENSOR_READ` | (none) | Emits current sensor readings immediately. |

### Events

| Event | Parameters | Description |
|---|---|---|
| `EVT:BOOT` | `fw=<name>`, `v=<ver>` | Emitted once on firmware startup. |
| `EVT:HEARTBEAT` | `status=OK`, `T=<temp>`, `H=<hum>`, `soil=<avg>`, `tank=<lvl>` | Periodic health status and sensor summary (every 1 s). |
| `EVT:SENSOR_DATA` | `temp=<n>`, `hum=<n>`, `ph=<n>`, `ec=<n>`, `level=<n>` | Full sensor dump (polled every 5 s or on demand). |
| `EVT:TANK_EMPTY` | `level_pct=<n>` | Critical alert when tank drops below 10%. Auto-stops pumps. |
| `EVT:PUMP_OVERCURRENT` | `amps=<n>` | Hard safety timeout reached for a pump, auto-stopping it. |
| `EVT:WATER_DONE` | (none) | Emitted when a watering cycle completes successfully. |

---

## Development Notes

- Implement HAL stubs in `src/protocol.cpp` (pump relay toggling, PWM writes, real sensor reads).
- Update `config/Config.h` to match your physical wiring and MCU choice.
- The firmware emits sensor data automatically every 10 s **and** on `SENSOR_READ` command.
- Heartbeat is emitted every 5 s. A missing heartbeat is treated as a controller fault by the backend.
- **Safety hard-limit**: `WATER_START` must enforce a maximum on-time guard in firmware. Never allow a pump to run indefinitely regardless of backend state.

See also: [07_Development/env_controller](../project-info/Docs/07_Development/env_controller/README.md)
