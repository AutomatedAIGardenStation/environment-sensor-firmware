#ifndef EVENT_GUARD_H
#define EVENT_GUARD_H

#include <stdint.h>
#include <Arduino.h>

/**
 * A stateful guard to prevent duplicate events from being emitted.
 */
class TankEmptyGuard {
public:
    TankEmptyGuard() : emptyState(false) {}

    /**
     * Checks if the event should be emitted.
     * @param isBelowThreshold true if the tank is currently below the empty threshold
     * @return true if this is a NEW transition to the empty state (event should be emitted)
     */
    bool check(bool isBelowThreshold) {
        if (isBelowThreshold) {
            if (!emptyState) {
                // Newly entered empty state
                emptyState = true;
                return true;
            }
        } else {
            // Tank is above threshold, reset state
            emptyState = false;
        }
        return false;
    }

private:
    bool emptyState;
};

#endif // EVENT_GUARD_H
