#include <unity.h>
#include <string.h>
#include "../../lib/actuators/Doser.h"
#include "../../lib/actuators/MockRelayDriver.h"
#include "../../src/protocol.h"

class MockRelayDriverDoser : public MockRelayDriver {};

static MockRelayDriverDoser* mockDriver = nullptr;
static Doser* doser = nullptr;

static char last_emitted_event[128] = {0};

void protocol_emit_event(const char* event) {
    strncpy(last_emitted_event, event, sizeof(last_emitted_event) - 1);
}

void setUp(void) {
    mockDriver = new MockRelayDriverDoser();
    doser = new Doser(mockDriver);
    last_emitted_event[0] = '\0';
}

void tearDown(void) {
    delete doser;
    delete mockDriver;
}

void test_doser_sequence() {
    uint32_t now = 1000;
    doser->startDose(100, 200, 300, 400, now);

    TEST_ASSERT_EQUAL_STRING("EVT:DOSE_STARTED", last_emitted_event);
    TEST_ASSERT_TRUE(mockDriver->valve_states[0]); // NutA on
    TEST_ASSERT_FALSE(mockDriver->valve_states[1]);

    // Tick within A
    now += 50;
    doser->tick(now);
    TEST_ASSERT_TRUE(mockDriver->valve_states[0]);

    // Transition to B
    now += 50; // total 100
    doser->tick(now);
    TEST_ASSERT_FALSE(mockDriver->valve_states[0]);
    TEST_ASSERT_TRUE(mockDriver->valve_states[1]);

    // Transition to pH_Up
    now += 200; // total 300
    doser->tick(now);
    TEST_ASSERT_FALSE(mockDriver->valve_states[1]);
    TEST_ASSERT_TRUE(mockDriver->valve_states[2]);

    // Transition to pH_Down
    now += 300; // total 600
    doser->tick(now);
    TEST_ASSERT_FALSE(mockDriver->valve_states[2]);
    TEST_ASSERT_TRUE(mockDriver->valve_states[3]);

    // Transition to IDLE
    now += 400; // total 1000
    doser->tick(now);
    TEST_ASSERT_FALSE(mockDriver->valve_states[3]);
    TEST_ASSERT_EQUAL_STRING("EVT:DOSE_COMPLETE", last_emitted_event);
}

void test_doser_skip_zero() {
    uint32_t now = 1000;
    doser->startDose(100, 0, 0, 200, now);

    TEST_ASSERT_TRUE(mockDriver->valve_states[0]); // NutA

    now += 100;
    doser->tick(now); // Should skip B and pH_Up right into pH_Down
    TEST_ASSERT_FALSE(mockDriver->valve_states[0]);
    TEST_ASSERT_FALSE(mockDriver->valve_states[1]);
    TEST_ASSERT_FALSE(mockDriver->valve_states[2]);
    TEST_ASSERT_TRUE(mockDriver->valve_states[3]);

    now += 200;
    doser->tick(now);
    TEST_ASSERT_FALSE(mockDriver->valve_states[3]);
    TEST_ASSERT_EQUAL_STRING("EVT:DOSE_COMPLETE", last_emitted_event);
}

void test_doser_abort() {
    uint32_t now = 1000;
    doser->startDose(100, 200, 300, 400, now);

    TEST_ASSERT_TRUE(mockDriver->valve_states[0]);

    doser->stop();

    TEST_ASSERT_FALSE(mockDriver->valve_states[0]);
    TEST_ASSERT_EQUAL_STRING("EVT:DOSE_ABORTED", last_emitted_event);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_doser_sequence);
    RUN_TEST(test_doser_skip_zero);
    RUN_TEST(test_doser_abort);
    return UNITY_END();
}
