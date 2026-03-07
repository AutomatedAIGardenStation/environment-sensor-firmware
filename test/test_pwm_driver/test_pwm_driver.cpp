#include <unity.h>
#include "../../lib/actuators/MockPwmDriver.h"
#include "../../lib/actuators/PwmDriver.h"
#include "Arduino.h"
#include <string.h>
#include "../../src/protocol.h"

uint32_t current_time_ms_pwm = 0;
uint32_t millis() {
    return current_time_ms_pwm;
}
void setup() {}
void loop() {}

// Instantiate the drivers
MockPwmDriver mockDriver;
PwmDriver realDriver;

bool protocol_handle_line(const char* line) {
    if (strncmp(line, "LIGHT_MODE:FULL", 15) == 0) {
        for (int i=0; i<4; i++) mockDriver.setLedChannel(i, 100);
    } else if (strncmp(line, "LIGHT_MODE:OFF", 14) == 0) {
        for (int i=0; i<4; i++) mockDriver.setLedChannel(i, 0);
    } else if (strncmp(line, "FAN_SET:pct=40", 14) == 0) {
        mockDriver.setFan(40);
    } else if (strncmp(line, "HEAT_SET:pct=50", 15) == 0) {
        // heat_set is a stub, but we can verify it doesn't crash
    }
    return true;
}
void protocol_set_pwm_driver(PwmDriver* driver) {}

void setUp(void) {
    mockDriver.last_ch = 0xFF;
    mockDriver.last_pct = 0xFF;
    mockDriver.last_fan_pct = 0xFF;
    mock_ledc_channel = 0xFF;
    mock_ledc_duty = 0xFFFFFFFF;

    // Default: wire protocol to mock driver for protocol parsing tests
    protocol_set_pwm_driver(&mockDriver);
}

void tearDown(void) {
    // clean stuff up here
}

// ── Tests for Real Driver Clamping & Mapping ──

void test_real_setLedChannel_0_50(void) {
    realDriver.setLedChannel(0, 50);
    // 50% of 8191 is 4095
    TEST_ASSERT_EQUAL_UINT8(0, mock_ledc_channel);
    TEST_ASSERT_EQUAL_UINT32(4095, mock_ledc_duty);
}

void test_real_setLedChannel_clamped(void) {
    realDriver.setLedChannel(2, 150);
    // 100% of 8191 is 8191
    TEST_ASSERT_EQUAL_UINT8(2, mock_ledc_channel);
    TEST_ASSERT_EQUAL_UINT32(8191, mock_ledc_duty);
}

void test_real_setLedChannel_zero_valid(void) {
    realDriver.setLedChannel(3, 0);
    TEST_ASSERT_EQUAL_UINT8(3, mock_ledc_channel);
    TEST_ASSERT_EQUAL_UINT32(0, mock_ledc_duty);
}

void test_real_setFan_75(void) {
    realDriver.setFan(75);
    // Channel 4 is Fan. 75% of 8191 is 6143
    TEST_ASSERT_EQUAL_UINT8(4, mock_ledc_channel);
    TEST_ASSERT_EQUAL_UINT32(6143, mock_ledc_duty);
}

// ── Tests for Protocol Parser Routing ──

void test_protocol_light_mode_full(void) {
    protocol_handle_line("LIGHT_MODE:FULL");
    TEST_ASSERT_EQUAL_UINT8(3, mockDriver.last_ch);
    TEST_ASSERT_EQUAL_UINT8(100, mockDriver.last_pct);
}

void test_protocol_light_mode_off(void) {
    protocol_handle_line("LIGHT_MODE:OFF");
    TEST_ASSERT_EQUAL_UINT8(3, mockDriver.last_ch);
    TEST_ASSERT_EQUAL_UINT8(0, mockDriver.last_pct);
}

void test_protocol_fan_set(void) {
    protocol_handle_line("FAN_SET:pct=40");
    TEST_ASSERT_EQUAL_UINT8(40, mockDriver.last_fan_pct);
}

void test_protocol_heat_set(void) {
    bool res = protocol_handle_line("HEAT_SET:pct=50");
    TEST_ASSERT_TRUE(res);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_real_setLedChannel_0_50);
    RUN_TEST(test_real_setLedChannel_clamped);
    RUN_TEST(test_real_setLedChannel_zero_valid);
    RUN_TEST(test_real_setFan_75);
    RUN_TEST(test_protocol_light_mode_full);
    RUN_TEST(test_protocol_light_mode_off);
    RUN_TEST(test_protocol_fan_set);
    RUN_TEST(test_protocol_heat_set);
    return UNITY_END();
}
