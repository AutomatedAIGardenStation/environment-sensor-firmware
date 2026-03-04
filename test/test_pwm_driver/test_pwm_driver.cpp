#include <unity.h>
#include "protocol.h"
#include "../../lib/actuators/MockPwmDriver.h"
#include "../../lib/actuators/PwmDriver.h"
#include "Arduino.h"

// Instantiate the drivers
MockPwmDriver mockDriver;
PwmDriver realDriver;

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

void test_protocol_light_set(void) {
    protocol_handle_line("LIGHT_SET:ch=2:pct=80");
    TEST_ASSERT_EQUAL_UINT8(2, mockDriver.last_ch);
    TEST_ASSERT_EQUAL_UINT8(80, mockDriver.last_pct);
}

void test_protocol_fan_set(void) {
    protocol_handle_line("FAN_SET:pct=40");
    TEST_ASSERT_EQUAL_UINT8(40, mockDriver.last_fan_pct);
}

void test_protocol_L1(void) {
    // L1 command calls all 4 channels at 100%
    protocol_handle_line("L1");
    // Since it's a loop 0 to 3, the last one called is ch=3
    TEST_ASSERT_EQUAL_UINT8(3, mockDriver.last_ch);
    TEST_ASSERT_EQUAL_UINT8(100, mockDriver.last_pct);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_real_setLedChannel_0_50);
    RUN_TEST(test_real_setLedChannel_clamped);
    RUN_TEST(test_real_setLedChannel_zero_valid);
    RUN_TEST(test_real_setFan_75);
    RUN_TEST(test_protocol_light_set);
    RUN_TEST(test_protocol_fan_set);
    RUN_TEST(test_protocol_L1);
    return UNITY_END();
}
