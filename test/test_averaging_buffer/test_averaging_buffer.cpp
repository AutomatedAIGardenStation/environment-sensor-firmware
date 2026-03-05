#include <unity.h>
#include "../../lib/util/AveragingBuffer.h"

// Including the Arduino.cpp from test_pwm_driver directly avoids linker issues
// since it has full mocks already defined for native testing.
#include "../test_pwm_driver/Arduino.cpp"

uint32_t millis() {
    return 0;
}
void loop() {}
void setup() {}

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_push_n_samples_average(void) {
    AveragingBuffer<uint16_t, 4> buffer;

    buffer.push(10);
    buffer.push(20);
    buffer.push(30);
    buffer.push(40);

    TEST_ASSERT_TRUE(buffer.isFull());
    TEST_ASSERT_EQUAL_FLOAT(25.0f, buffer.average());
}

void test_push_fewer_than_n_samples(void) {
    AveragingBuffer<uint16_t, 4> buffer;

    buffer.push(10);
    buffer.push(20);

    TEST_ASSERT_FALSE(buffer.isFull());
    TEST_ASSERT_EQUAL_FLOAT(15.0f, buffer.average());
}

void test_push_n_plus_one_samples(void) {
    AveragingBuffer<uint16_t, 4> buffer;

    buffer.push(10);
    buffer.push(20);
    buffer.push(30);
    buffer.push(40);

    // Buffer is full. Push 50, replacing 10.
    buffer.push(50);

    // Elements are now 20, 30, 40, 50. Mean = 35.
    TEST_ASSERT_TRUE(buffer.isFull());
    TEST_ASSERT_EQUAL_FLOAT(35.0f, buffer.average());
}

void test_all_identical_samples(void) {
    AveragingBuffer<uint16_t, 5> buffer;

    for (int i = 0; i < 5; i++) {
        buffer.push(42);
    }

    TEST_ASSERT_TRUE(buffer.isFull());
    TEST_ASSERT_EQUAL_FLOAT(42.0f, buffer.average());
}

void test_buffer_wraps_correctly(void) {
    AveragingBuffer<uint16_t, 3> buffer;

    // Push 2xN samples
    buffer.push(10);
    buffer.push(20);
    buffer.push(30);

    buffer.push(40);
    buffer.push(50);
    buffer.push(60);

    // Elements should now be 40, 50, 60. Mean = 50.
    TEST_ASSERT_TRUE(buffer.isFull());
    TEST_ASSERT_EQUAL_FLOAT(50.0f, buffer.average());
}

void test_empty_buffer(void) {
    AveragingBuffer<uint16_t, 4> buffer;

    TEST_ASSERT_FALSE(buffer.isFull());
    TEST_ASSERT_EQUAL_FLOAT(0.0f, buffer.average());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_push_n_samples_average);
    RUN_TEST(test_push_fewer_than_n_samples);
    RUN_TEST(test_push_n_plus_one_samples);
    RUN_TEST(test_all_identical_samples);
    RUN_TEST(test_buffer_wraps_correctly);
    RUN_TEST(test_empty_buffer);
    UNITY_END();

    return 0;
}
