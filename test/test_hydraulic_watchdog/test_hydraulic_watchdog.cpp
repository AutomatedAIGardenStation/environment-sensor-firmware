#include <unity.h>
#include "../../lib/actuators/MockRelayDriver.h"
#include "../../lib/actuators/HydraulicWatchdog.h"
#include "../../src/protocol.h"
#include <string.h>
#include "../test_pwm_driver/Arduino.h"

// Pull in the Arduino.cpp implementations for native test
#include "../test_pwm_driver/Arduino.cpp"

// Mocks
static MockRelayDriver* mockDriver = nullptr;
static HydraulicWatchdog* watchdog = nullptr;
static uint32_t current_time_ms = 0;

uint32_t millis() {
    return current_time_ms;
}
void setup() {}
void loop() {}

// Mocking the event correctly
extern char last_emitted_event[256];

void protocol_set_relay_driver(IRelayDriver* driver) {}
void protocol_set_watchdog(HydraulicWatchdog* wd) {}
void protocol_emit_event(const char* event) {
    strncpy(last_emitted_event, event, 255);
    last_emitted_event[255] = '\0';
}
bool protocol_handle_line(const char* line) {
    if (strncmp(line, "WATER_START:zone=1", 18) == 0) {
        mockDriver->setRelay(1, true);
        watchdog->start(1, current_time_ms);
    } else if (strncmp(line, "WATER_STOP", 10) == 0) {
        for (int i=0; i<4; i++) mockDriver->setRelay(i, false);
        watchdog->stop();
        protocol_emit_event("EVT:WATER_DONE");
    } else if (strncmp(line, "W1", 2) == 0) {
        mockDriver->setRelay(0, true);
        watchdog->start(0, current_time_ms);
    }
    return true;
}

void setUp(void) {
    mockDriver = new MockRelayDriver();
    watchdog = new HydraulicWatchdog(mockDriver);
    last_emitted_event[0] = '\0';
    current_time_ms = 0;

    // Wire protocol
    protocol_set_relay_driver(mockDriver);
    protocol_set_watchdog(watchdog);
}

void tearDown(void) {
    delete watchdog;
    delete mockDriver;
}

void test_watchdog_before_timeout_pump_stays_on(void) {
    current_time_ms = 1000;
    mockDriver->setRelay(0, true);
    watchdog->start(0, current_time_ms);

    current_time_ms += (MAX_PUMP_TIME_MS - 1);
    watchdog->tick(current_time_ms);

    TEST_ASSERT_TRUE(mockDriver->pump_states[0]);
    TEST_ASSERT_EQUAL_STRING("", last_emitted_event);
}

void test_watchdog_exactly_at_timeout_turns_off(void) {
    current_time_ms = 1000;
    mockDriver->setRelay(0, true);
    watchdog->start(0, current_time_ms);

    current_time_ms += MAX_PUMP_TIME_MS;
    watchdog->tick(current_time_ms);

    TEST_ASSERT_FALSE(mockDriver->pump_states[0]);
    TEST_ASSERT_EQUAL_STRING(EVT_PUMP_OVERCURRENT ":amps=0", last_emitted_event);
}

void test_watchdog_after_timeout_no_double_emit(void) {
    current_time_ms = 1000;
    mockDriver->setRelay(0, true);
    watchdog->start(0, current_time_ms);

    current_time_ms += MAX_PUMP_TIME_MS;
    watchdog->tick(current_time_ms);

    // Clear last event to test next tick
    last_emitted_event[0] = '\0';

    current_time_ms += 1;
    watchdog->tick(current_time_ms);

    TEST_ASSERT_FALSE(mockDriver->pump_states[0]);
    TEST_ASSERT_EQUAL_STRING("", last_emitted_event);
}

void test_watchdog_stop_before_timeout_cancels(void) {
    current_time_ms = 1000;
    mockDriver->setRelay(0, true);
    watchdog->start(0, current_time_ms);

    current_time_ms += (MAX_PUMP_TIME_MS - 100);
    watchdog->stop();

    current_time_ms += 200; // Now past original timeout
    watchdog->tick(current_time_ms);

    TEST_ASSERT_TRUE(mockDriver->pump_states[0]); // Left as is
    TEST_ASSERT_EQUAL_STRING("", last_emitted_event);
}

void test_water_start_then_stop_protocol(void) {
    current_time_ms = 1000;
    protocol_handle_line("WATER_START:zone=1");

    TEST_ASSERT_TRUE(mockDriver->pump_states[1]);

    current_time_ms += (MAX_PUMP_TIME_MS - 100);
    protocol_handle_line("WATER_STOP");

    TEST_ASSERT_FALSE(mockDriver->pump_states[1]);
    TEST_ASSERT_EQUAL_STRING(EVT_WATER_DONE, last_emitted_event);

    current_time_ms += 200; // Now past original timeout
    watchdog->tick(current_time_ms);

    TEST_ASSERT_FALSE(mockDriver->pump_states[1]);
    TEST_ASSERT_EQUAL_STRING(EVT_WATER_DONE, last_emitted_event); // No overcurrent
}

void test_w1_protocol(void) {
    current_time_ms = 1000;
    protocol_handle_line("W1");

    TEST_ASSERT_TRUE(mockDriver->pump_states[0]);

    current_time_ms += MAX_PUMP_TIME_MS;
    watchdog->tick(current_time_ms);

    TEST_ASSERT_FALSE(mockDriver->pump_states[0]);
    TEST_ASSERT_EQUAL_STRING(EVT_PUMP_OVERCURRENT ":amps=0", last_emitted_event);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_watchdog_before_timeout_pump_stays_on);
    RUN_TEST(test_watchdog_exactly_at_timeout_turns_off);
    RUN_TEST(test_watchdog_after_timeout_no_double_emit);
    RUN_TEST(test_watchdog_stop_before_timeout_cancels);
    RUN_TEST(test_water_start_then_stop_protocol);
    RUN_TEST(test_w1_protocol);
    return UNITY_END();
}
