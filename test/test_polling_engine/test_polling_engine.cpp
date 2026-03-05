#include <unity.h>
#include "../../lib/scheduler/PollingEngine.h"

// Stubs to resolve compilation link issues with `test_build_src = yes`
void loop() {}
void setup() {}
uint32_t millis() { return 0; }
int analogRead(uint8_t pin) { return 0; }
void analogWrite(uint8_t pin, int val) {}
void digitalWrite(uint8_t pin, uint8_t val) {}
void pinMode(uint8_t pin, uint8_t mode) {}
void ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits) {}
void ledcAttachPin(uint8_t pin, uint8_t channel) {}
void ledcWrite(uint8_t channel, uint32_t duty) {}

class MockSerialPE {
public:
    void begin(int baud) {}
    int available() { return 0; }
    int read() { return -1; }
    void print(const char* s) {}
    void println(const char* s) {}
    operator bool() const { return true; }
};

MockSerialPE Serial;

int callback1_count = 0;
int callback2_count = 0;

void callback1() {
    callback1_count++;
}

void callback2() {
    callback2_count++;
}

void setUp(void) {
    callback1_count = 0;
    callback2_count = 0;
}

void tearDown(void) {
    // clean stuff up here
}

void test_task_not_yet_due(void) {
    PollingEngine engine;
    engine.addTask("test1", 1000, callback1);

    engine.tick(0); // initialize
    engine.tick(500); // not yet due

    TEST_ASSERT_EQUAL_INT(0, callback1_count);
}

void test_task_exactly_at_interval(void) {
    PollingEngine engine;
    engine.addTask("test1", 1000, callback1);

    engine.tick(0);
    engine.tick(1000); // exactly due

    TEST_ASSERT_EQUAL_INT(1, callback1_count);
}

void test_task_overdue_called_once(void) {
    PollingEngine engine;
    engine.addTask("test1", 1000, callback1);

    engine.tick(0);
    engine.tick(2500); // overdue by 2x interval

    TEST_ASSERT_EQUAL_INT(1, callback1_count); // should only call once per tick
}

void test_multiple_tasks_different_intervals(void) {
    PollingEngine engine;
    engine.addTask("test1", 1000, callback1);
    engine.addTask("test2", 500, callback2);

    engine.tick(0);
    engine.tick(400);

    // neither due
    TEST_ASSERT_EQUAL_INT(0, callback1_count);
    TEST_ASSERT_EQUAL_INT(0, callback2_count);

    engine.tick(500);
    // test2 due
    TEST_ASSERT_EQUAL_INT(0, callback1_count);
    TEST_ASSERT_EQUAL_INT(1, callback2_count);

    engine.tick(1000);
    // both due
    TEST_ASSERT_EQUAL_INT(1, callback1_count);
    TEST_ASSERT_EQUAL_INT(2, callback2_count);

    engine.tick(1500);
    // test2 due again
    TEST_ASSERT_EQUAL_INT(1, callback1_count);
    TEST_ASSERT_EQUAL_INT(3, callback2_count);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_task_not_yet_due);
    RUN_TEST(test_task_exactly_at_interval);
    RUN_TEST(test_task_overdue_called_once);
    RUN_TEST(test_multiple_tasks_different_intervals);
    return UNITY_END();
}
