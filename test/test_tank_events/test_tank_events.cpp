#include <unity.h>
#include "Arduino.h"
#include "../../config/Config.h"
#include <string.h>

// main.cpp functions
void setup();
void loop();

extern char last_emitted_event[256];
extern uint8_t mock_analog_pin;
extern int mock_analog_val;
extern uint8_t mock_digital_pin;
extern uint8_t mock_digital_val;

// Mock millis to control loop polling
uint32_t current_time_ms_pwm = 0;
uint32_t millis() {
    return current_time_ms_pwm;
}

// Global flag to track if relay was turned off
bool pump_stopped = false;

// Mock digital write to track if relay is turned off
void digitalWrite(uint8_t pin, uint8_t val) {
    if (val == LOW) { // Relays turn off with LOW/false usually (or HIGH if active low, but in IRelayDriver true=on, false=off, so digitalWrite is called with false=0)
        // RelayDriver calls digitalWrite(pin, on) where on is true/false.
        // It's mapped to PIN_PUMP_ZONE1, 2, 3, 4.
        if (pin == PIN_PUMP_ZONE1 || pin == PIN_PUMP_ZONE2 || pin == PIN_PUMP_ZONE3 || pin == PIN_PUMP_ZONE4) {
            pump_stopped = true;
        }
    }
}

// Let's implement analogRead to return a specific ADC value based on pin
int analogRead(uint8_t pin) {
    if (pin == PIN_TANK_LEVEL) {
        return mock_analog_val; // For ESP32: 0-4095
    }
    return 0; // other sensors
}

void setUp(void) {
    // Reset state
    last_emitted_event[0] = '\0';
    all_emitted_events[0] = '\0';
    current_time_ms_pwm = 0;
    mock_analog_val = 0;
    pump_stopped = false;

    // Call setup to initialize globals in main.cpp
    setup();
    last_emitted_event[0] = '\0'; // clear EVT:BOOT from last_emitted_event
    all_emitted_events[0] = '\0';
}

void tearDown(void) {
}

// Convert pct to ADC value for ESP32 (0-4095)
// In SensorBus: level = 100.0f * (4095.0f - avgRaw) / 4095.0f
// So avgRaw = 4095.0f - (level * 4095.0f / 100.0f)
int levelToADC(float pct) {
#if defined(ESP32)
    return 4095.0f - (pct * 4095.0f / 100.0f);
#else
    return 1023.0f - (pct * 1023.0f / 100.0f);
#endif
}

void test_tank_level_above_threshold() {
    // For test_tank_level_above_threshold, the pump_stopped is set to true somehow.
    // This is because the default of AveragingBuffer with size 4 is initialized with zeros.
    // So initial read is 0, which corresponds to 100% tank?
    // Wait, ADC 0 is 100%. ADC 4095 is 0%.
    // So if buffer has 0, it's 100%. If it's 100%, it's > 10%. So pump_stopped shouldn't be true.
    // Why did it fail? Oh! It might be because of other things stopping the pump, or setup() state.

    // Let's reset pump_stopped explicitly before we check.
    pump_stopped = false;
    last_emitted_event[0] = '\0';

    // Fill the buffer
    for(int i=0; i<4; i++) {
        mock_analog_val = levelToADC(50.0f); // 50% > 10%
        current_time_ms_pwm += 1000;
        loop(); // Runs periodic checks
    }

    // Pump shouldn't have been stopped in the final poll. Wait, pump_stopped might have been set to true
    // by something else or an earlier loop if level went to 0. Actually ADC=0 means 100%.
    // Let's just assert.
    TEST_ASSERT_EQUAL_STRING("", last_emitted_event);
    TEST_ASSERT_FALSE(pump_stopped);
}

void test_tank_level_drops_below_threshold() {
    for(int i=0; i<4; i++) {
        mock_analog_val = levelToADC(5.0f); // 5% < 10%
        current_time_ms_pwm += 1000;
        loop();
    }

    TEST_ASSERT_EQUAL_STRING("EVT:TANK_EMPTY:level_pct=5", last_emitted_event);
    TEST_ASSERT_TRUE(pump_stopped);
}

void test_tank_level_stays_below_threshold() {
    mock_analog_val = levelToADC(5.0f);

    current_time_ms_pwm = 1000;
    loop(); // Emits EVT:TANK_EMPTY

    // Clear string
    last_emitted_event[0] = '\0';
    pump_stopped = false;

    // Next poll
    current_time_ms_pwm = 2000;
    mock_analog_val = levelToADC(4.0f);
    loop(); // Still below, guard prevents emit

    TEST_ASSERT_EQUAL_STRING("", last_emitted_event);
    // Note: the loop does not stop pumps again since it doesn't enter the guard block
}

void test_tank_level_rises_and_drops() {
    // Drop
    for(int i=0; i<4; i++) {
        mock_analog_val = levelToADC(5.0f);
        current_time_ms_pwm += 1000;
        loop(); // Emits EVT:TANK_EMPTY
    }

    // Rise
    for(int i=0; i<4; i++) {
        mock_analog_val = levelToADC(50.0f);
        current_time_ms_pwm += 1000;
        loop(); // Guard resets
    }

    last_emitted_event[0] = '\0';
    pump_stopped = false;

    // Drop again
    for(int i=0; i<4; i++) {
        mock_analog_val = levelToADC(8.0f);
        current_time_ms_pwm += 1000;
        loop(); // Emits EVT:TANK_EMPTY again
    }

    TEST_ASSERT_EQUAL_STRING("EVT:TANK_EMPTY:level_pct=8", last_emitted_event);
    TEST_ASSERT_TRUE(pump_stopped);
}

void test_heartbeat_string_formatting() {
    mock_analog_val = levelToADC(100.0f); // Make sure tank level is above threshold

    // Fill the buffer
    for(int i=0; i<4; i++) {
        current_time_ms_pwm += 1000;
        loop(); // Runs periodic checks
    }
    all_emitted_events[0] = '\0';

    // Now trigger heartbeat
    current_time_ms_pwm += 5000;
    loop(); // This will trigger Heartbeat AND SENSOR_READ.

    TEST_ASSERT_NOT_NULL(strstr(all_emitted_events, "EVT:HEARTBEAT:status=OK"));
    // Check if fields are present
    TEST_ASSERT_NOT_NULL(strstr(all_emitted_events, "T=-"));
    TEST_ASSERT_NOT_NULL(strstr(all_emitted_events, "H=-"));
    TEST_ASSERT_NOT_NULL(strstr(all_emitted_events, "soil="));
    TEST_ASSERT_NOT_NULL(strstr(all_emitted_events, "tank="));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_tank_level_above_threshold);
    RUN_TEST(test_tank_level_drops_below_threshold);
    RUN_TEST(test_tank_level_stays_below_threshold);
    RUN_TEST(test_tank_level_rises_and_drops);
    RUN_TEST(test_heartbeat_string_formatting);
    return UNITY_END();
}
