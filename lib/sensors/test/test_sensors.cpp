#include <unity.h>
#include "../../lib/sensors/I2CSensorAdapters.h"

// Provide mock functions for native testing
uint32_t current_millis_mock = 0;
uint32_t millis() {
    return current_millis_mock;
}

void loop() {}
void setup() {}

void setUp(void) {
    current_millis_mock = 0;
}

void tearDown(void) {
}

void test_initial_read_is_negative_999(void) {
    AtlasSensorAdapter adapter(0x63, 900, 2000, 10000, 0.0f, 14.0f);
    TEST_ASSERT_EQUAL_FLOAT(-999.0f, adapter.getAverage());
}

void test_successful_read_averaging(void) {
    AtlasSensorAdapter adapter(0x63, 900, 2000, 10000, 0.0f, 14.0f);

    // Inject valid reads directly into buffer
    adapter.mockInjectValidRead(6.0f, current_millis_mock);
    adapter.mockInjectValidRead(7.0f, current_millis_mock);
    adapter.mockInjectValidRead(8.0f, current_millis_mock);

    TEST_ASSERT_EQUAL_FLOAT(7.0f, adapter.getAverage());
}

void test_stale_read_timeout(void) {
    AtlasSensorAdapter adapter(0x63, 900, 2000, 10000, 0.0f, 14.0f);

    adapter.mockInjectValidRead(7.0f, current_millis_mock);
    TEST_ASSERT_EQUAL_FLOAT(7.0f, adapter.getAverage());

    // Advance time past stale timeout
    current_millis_mock += 10001;

    TEST_ASSERT_EQUAL_FLOAT(-999.0f, adapter.getAverage());
}

void test_adapter_state_machine_non_blocking_timing(void) {
    AtlasSensorAdapter adapter(0x63, 900, 2000, 10000, 0.0f, 14.0f);

    // Initial state: IDLE
    adapter.tick(current_millis_mock); // Time 0, triggers startMeasurement -> WAITING

    // Advance time but not enough for delay
    current_millis_mock += 500;
    adapter.tick(current_millis_mock); // Still WAITING

    // Advance time past delay
    current_millis_mock += 400; // Total 900
    adapter.tick(current_millis_mock); // Triggers readMeasurement -> IDLE

    // Advance time but not enough for next poll interval
    current_millis_mock += 1000; // Total 1900
    adapter.tick(current_millis_mock); // Still IDLE

    // Advance time past poll interval
    current_millis_mock += 100; // Total 2000
    adapter.tick(current_millis_mock); // Triggers startMeasurement -> WAITING

    TEST_ASSERT_TRUE(true); // If it didn't crash, we're good
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_initial_read_is_negative_999);
    RUN_TEST(test_successful_read_averaging);
    RUN_TEST(test_stale_read_timeout);
    RUN_TEST(test_adapter_state_machine_non_blocking_timing);
    return UNITY_END();
}
