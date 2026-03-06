#include "Doser.h"
#include "../../src/protocol.h"
#include "../../config/Config.h"
#include <stdio.h>

Doser::Doser(IRelayDriver* driver) : driver(driver), currentState(IDLE), start_ms(0),
                                     timeA(0), timeB(0), timePhUp(0), timePhDown(0) {}

void Doser::startDose(uint32_t msA, uint32_t msB, uint32_t msPhUp, uint32_t msPhDown, uint32_t now_ms) {
    if (currentState != IDLE) {
        stop(); // abort current
    }

    timeA = msA;
    timeB = msB;
    timePhUp = msPhUp;
    timePhDown = msPhDown;

    protocol_emit_event("EVT:DOSE_STARTED");

    transitionTo(DOSING_A, now_ms);
}

void Doser::stop() {
    if (currentState != IDLE) {
        if (driver) {
            driver->setValve(0, false); // NutA
            driver->setValve(1, false); // NutB
            driver->setValve(2, false); // pH_Up
            driver->setValve(3, false); // pH_Down
        }
        currentState = IDLE;
        protocol_emit_event("EVT:DOSE_ABORTED");
    }
}

void Doser::transitionTo(DoseState nextState, uint32_t now_ms) {
    if (driver) {
        driver->setValve(0, false);
        driver->setValve(1, false);
        driver->setValve(2, false);
        driver->setValve(3, false);
    }

    start_ms = now_ms;

    // Skip states with 0 time
    if (nextState == DOSING_A && timeA == 0) nextState = DOSING_B;
    if (nextState == DOSING_B && timeB == 0) nextState = DOSING_PH_UP;
    if (nextState == DOSING_PH_UP && timePhUp == 0) nextState = DOSING_PH_DOWN;
    if (nextState == DOSING_PH_DOWN && timePhDown == 0) nextState = IDLE;

    currentState = nextState;

    if (currentState == DOSING_A && driver) {
        driver->setValve(0, true);
    } else if (currentState == DOSING_B && driver) {
        driver->setValve(1, true);
    } else if (currentState == DOSING_PH_UP && driver) {
        driver->setValve(2, true);
    } else if (currentState == DOSING_PH_DOWN && driver) {
        driver->setValve(3, true);
    } else if (currentState == IDLE) {
        // Also emit NUTRIENT_UPDATE to satisfy AC
        char buf[64];
        // simple ml conversion placeholder (e.g. 1ms = 0.01ml)
        float mlA = timeA * 0.01f;
        float mlB = timeB * 0.01f;
        snprintf(buf, sizeof(buf), "EVT:NUTRIENT_UPDATE:NutA=%.1f:NutB=%.1f", mlA, mlB);
        protocol_emit_event(buf);

        protocol_emit_event("EVT:DOSE_COMPLETE");
    }
}

void Doser::tick(uint32_t now_ms) {
    if (currentState == IDLE) return;

    uint32_t elapsed = now_ms - start_ms;

    if (currentState == DOSING_A && elapsed >= timeA) {
        transitionTo(DOSING_B, now_ms);
    } else if (currentState == DOSING_B && elapsed >= timeB) {
        transitionTo(DOSING_PH_UP, now_ms);
    } else if (currentState == DOSING_PH_UP && elapsed >= timePhUp) {
        transitionTo(DOSING_PH_DOWN, now_ms);
    } else if (currentState == DOSING_PH_DOWN && elapsed >= timePhDown) {
        transitionTo(IDLE, now_ms);
    }
}
