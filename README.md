# env_controller

Firmware for the **environmental subsystem** of Garden Station.

Handles: single submersible pump, solenoid valve manifold (5 nutrient/pH valves), 4-channel LED grow lights, fan, heater, and all environmental sensors (temperature, humidity, pH, EC, water level, light, CO2).

> **Architecture**: The backend sends commands to this controller via serial. Sensor telemetry is pushed over Wi-Fi (MQTT/TCP) to the backend's Network Telemetry Listener — there is no direct USB tether for telemetry data.
>
> Full protocol spec: `project-info/Docs/06_Software/env_controller/Serial Protocol & Interfaces.md`

## Module Contract

| | |
|---|---|
| **Input** | Valve toggles, main pump toggle, light/fan/heater PWM commands (via serial) |
| **Output** | `EVT:SENSOR_UPDATE`, `EVT:AIR_TEMP`, `EVT:AIR_HUMIDITY`, `EVT:NUTRIENT_UPDATE`, threshold events (pushed over Wi-Fi MQTT/TCP) |
| **Constraint** | Hardware-agnostic MCU. No direct USB tether to backend for telemetry. Commands arrive via serial; telemetry exits via Wi-Fi. |

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

```text
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
| `PUMP_RUN` | `ms=<time>` | Run the single submersible pump for a duration in ms |
| `PUMP_MAIN` | `state=<0\|1>` | Direct pump on/off toggle (backend manages timing) |
| `WATER_STOP` | (none) | Emergency stop — stop pump immediately |
| `VALVE_SET` | `id=<name>`, `state=<0\|1>` | Toggle individual solenoid valve (NutA, NutB, pH_Up, pH_Down, CO2) |
| `DOSE_RECIPE` | `v1=<ms>:v2=<ms>:v3=<ms>:v4=<ms>:v5=<ms>` | Execute multi-nutrient dosing recipe via solenoid manifold |
| `DOSE_STOP` | (none) | Emergency stop — close all valves, stop pump |
| `LIGHT_SET` | `ch=<n>`, `pct=<0-100>` | Set LED channel PWM (1=Red, 2=Blue, 3=White, 4=Far-Red) |
| `LIGHT_MODE` | `<preset>` | Apply spectrum preset (SEEDLING, VEG, BLOOM, FULL, OFF) |
| `FAN_SET` | `pct=<0-100>` | Set fan speed percentage (0 = off) |
| `HEAT_SET` | `pct=<0-100>` | Set heating element power via SSR PWM (0 = off) |

### Events

#### Telemetry Events (pushed over Wi-Fi MQTT/TCP)

| Event | Description |
|---|---|
| `EVT:SENSOR_UPDATE:temp=<n>:humidity=<n>:water_level_pct=<n>:light_lux=<n>` | Periodic environmental snapshot |
| `EVT:AIR_TEMP:temp=<C>` | Air temperature reading |
| `EVT:AIR_HUMIDITY:humidity=<%RH>` | Air humidity reading |
| `EVT:NUTRIENT_UPDATE:ec=<n>:ph=<n>:co2=<n>` | Periodic nutrient + CO2 snapshot |
| `EVT:LIGHT_STATE:ch1=<pct>:ch2=<pct>:ch3=<pct>:ch4=<pct>` | LED channel state on change |

#### Threshold Events (trigger AI decision pipeline)

| Event | Description |
|---|---|
| `EVT:EC_LOW:val=<n>` | EC below plant profile minimum |
| `EVT:EC_HIGH:val=<n>` | EC above plant profile maximum |
| `EVT:PH_LOW:val=<n>` | pH too acidic |
| `EVT:PH_HIGH:val=<n>` | pH too alkaline |
| `EVT:CO2_LOW:val=<n>` | CO2 below threshold |
| `EVT:TEMP_LOW:val=<n>` | Temperature below threshold |
| `EVT:TEMP_HIGH:val=<n>` | Temperature above threshold |
| `EVT:SOIL_DRY:val=<n>` | Soil moisture below threshold |

#### Fault & Safety Events

| Event | Description |
|---|---|
| `EVT:BOOT:fw=<name>:v=<ver>` | Emitted once on firmware startup |
| `EVT:WATER_DONE` | Watering / pump cycle completed |
| `EVT:TANK_EMPTY:level_pct=<n>` | Reservoir empty — watering must stop |
| `EVT:PUMP_OVERCURRENT:amps=<n>` | Pump overcurrent — relay cut by firmware |
| `EVT:HEAT_OVERCURRENT:amps=<n>` | Heater overcurrent — relay cut by firmware |
| `EVT:HEARTBEAT:status=OK` | Periodic heartbeat |

#### Dosing Lifecycle Events

| Event | Description |
|---|---|
| `EVT:DOSE_START:recipe_seq=<n>` | Recipe execution begun |
| `EVT:DOSE_STEP:valve=<n>:duration=<ms>` | Valve open, pump running for duration |
| `EVT:DOSE_COMPLETE:total_ms=<n>` | All recipe steps completed |
| `EVT:DOSE_ABORT:reason=<code>` | Dosing aborted (OVERCURRENT, TANK_EMPTY, MANUAL_STOP, TIMEOUT) |

---

## Development Notes

- Implement HAL stubs in `src/protocol.cpp` (pump relay toggling, valve relay toggling, PWM writes, real sensor reads).
- Update `config/Config.h` to match your physical wiring and MCU choice.
- The firmware emits sensor telemetry automatically on a configurable timer (default: every 30 s) and on threshold crossings.
- Heartbeat is emitted every 1 s. A missing heartbeat is treated as a controller fault by the backend.
- **Safety hard-limit**: `PUMP_RUN` and `DOSE_RECIPE` must enforce a maximum on-time guard in firmware (default: 30 s). Never allow a pump to run indefinitely regardless of backend state.

See also: [07_Development/env_controller](../project-info/Docs/07_Development/env_controller/README.md)
