#ifndef DOSER_H
#define DOSER_H

#include <stdint.h>
#include "../hal/IRelayDriver.h"

class Doser {
public:
    Doser(IRelayDriver* driver);

    void startDose(uint32_t msA, uint32_t msB, uint32_t msPhUp, uint32_t msPhDown, uint32_t now_ms);
    void stop();
    void tick(uint32_t now_ms);

private:
    IRelayDriver* driver;

    enum DoseState {
        IDLE,
        DOSING_A,
        DOSING_B,
        DOSING_PH_UP,
        DOSING_PH_DOWN
    };

    DoseState currentState;
    uint32_t start_ms;

    uint32_t timeA;
    uint32_t timeB;
    uint32_t timePhUp;
    uint32_t timePhDown;

    void transitionTo(DoseState nextState, uint32_t now_ms);
};

#endif // DOSER_H
