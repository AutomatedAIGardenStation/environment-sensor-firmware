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
```

Available environments: `nanoatmega328`, `uno`, `esp32dev`

---

## Project Structure

```
env_controller/
├── platformio.ini      # PlatformIO environments for supported boards
├── config/
│   └── Config.h        # Board-specific pin assignments and global constants
└── src/
    ├── main.cpp        # Setup / loop, Serial reader, heartbeat, sensor polling
    ├── protocol.h      # Command & event constants, public API
    └── protocol.cpp    # Command dispatcher – stub HAL calls to replace
```

---

## Serial Protocol (summary)

| Direction | Format |
|-----------|--------|
| Backend → Controller | `<COMMAND>[:<key>=<val>]*\n` |
| Controller → Backend | `EVT:<NAME>[:<key>=<val>]*\n` |

**Key commands:** `WATER_START:zone=<id>`, `WATER_STOP`, `F1` (feed), `LIGHT_SET:ch=<n>:pct=<0-100>`, `FAN_SET:pct=<0-100>`, `SENSOR_READ`

**Key events:** `EVT:WATER_DONE`, `EVT:TANK_LOW:level_pct=<n>`, `EVT:TANK_EMPTY:level_pct=<n>`, `EVT:PUMP_OVERCURRENT:amps=<n>`, `EVT:SENSOR_DATA:temp=<n>:hum=<n>:ph=<n>:ec=<n>:level=<n>`, `EVT:HEARTBEAT:status=OK`

---

## Development Notes

- Implement HAL stubs in `src/protocol.cpp` (pump relay toggling, PWM writes, real sensor reads).
- Update `config/Config.h` to match your physical wiring and MCU choice.
- The firmware emits sensor data automatically every 10 s **and** on `SENSOR_READ` command.
- Heartbeat is emitted every 5 s. A missing heartbeat is treated as a controller fault by the backend.
- **Safety hard-limit**: `WATER_START` must enforce a maximum on-time guard in firmware. Never allow a pump to run indefinitely regardless of backend state.

See also: [07_Development/env_controller](../project-info/Docs/07_Development/env_controller/README.md)
