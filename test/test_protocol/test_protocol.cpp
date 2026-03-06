#include <unity.h>
#include <string.h>
#include "../../src/protocol.h"
#include "../../lib/actuators/PwmDriver.h"
#include "../../lib/actuators/HydraulicWatchdog.h"
#include "../../lib/actuators/Doser.h"
#include "../../lib/hal/IRelayDriver.h"
#include "Arduino.h"

// Stubs for protocol.cpp to compile
void loop() {}
void setup() {}
int analogRead(uint8_t pin) { return 0; }
void analogWrite(uint8_t pin, int val) {}
void digitalWrite(uint8_t pin, uint8_t val) {}
void pinMode(uint8_t pin, uint8_t mode) {}
void ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits) {}
void ledcAttachPin(uint8_t pin, uint8_t channel) {}
void ledcWrite(uint8_t channel, uint32_t duty) {}

// Include protocol.cpp directly to test its static logic in this suite
#include "../../src/protocol.cpp"

void setUp(void) {
    Serial.output = "";
}

void tearDown(void) {
}

void test_valid_command_pump_run(void) {
    // 0123:PUMP_RUN:ms=500:
    // crc8 of "0123:PUMP_RUN:ms=500:"
    const char* str_to_crc = "0123:PUMP_RUN:ms=500:";
    uint8_t expected_crc = crc8(reinterpret_cast<const uint8_t*>(str_to_crc), strlen(str_to_crc));

    char buf[128];
    snprintf(buf, sizeof(buf), "0123:PUMP_RUN:ms=500:CRC=%02X", expected_crc);

    bool res = protocol_handle_line(buf);
    TEST_ASSERT_TRUE(res);
    TEST_ASSERT_EQUAL_STRING("ACK:0123\n", Serial.output.c_str());
}

void test_invalid_crc(void) {
    const char* str = "ABCD:PUMP_STOP:CRC=00";

    bool res = protocol_handle_line(str);
    TEST_ASSERT_FALSE(res);
    TEST_ASSERT_EQUAL_STRING("NACK:ABCD:BADCRC\n", Serial.output.c_str());
}

void test_unknown_command(void) {
    const char* str_to_crc = "1111:FAKE_CMD:";
    uint8_t expected_crc = crc8(reinterpret_cast<const uint8_t*>(str_to_crc), strlen(str_to_crc));

    char buf[128];
    snprintf(buf, sizeof(buf), "1111:FAKE_CMD:CRC=%02X", expected_crc);

    bool res = protocol_handle_line(buf);
    TEST_ASSERT_FALSE(res);
    TEST_ASSERT_EQUAL_STRING("NACK:1111:UNKNOWN\n", Serial.output.c_str());
}

void test_missing_crc(void) {
    const char* str = "1234:PUMP_RUN";

    bool res = protocol_handle_line(str);
    TEST_ASSERT_FALSE(res);
    // When there's no CRC, the parser should fail looking for :CRC=
    TEST_ASSERT_EQUAL_STRING("NACK:1234:BADCRC\n", Serial.output.c_str());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_valid_command_pump_run);
    RUN_TEST(test_invalid_crc);
    RUN_TEST(test_unknown_command);
    RUN_TEST(test_missing_crc);
    return UNITY_END();
}
