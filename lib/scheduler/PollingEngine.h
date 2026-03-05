#ifndef POLLING_ENGINE_H
#define POLLING_ENGINE_H

#include <stdint.h>
#include <stddef.h>

#define MAX_POLLING_TASKS 10

typedef void (*PollingCallback)();

struct PollingTask {
    const char* name;
    uint32_t interval_ms;
    PollingCallback callback;
    uint32_t last_run_ms;
    bool active;
};

class PollingEngine {
public:
    PollingEngine();

    // Adds a task to the polling engine. Returns true if successful, false if the engine is full.
    bool addTask(const char* name, uint32_t interval_ms, PollingCallback callback);

    // Ticks the engine, calling tasks whose intervals have elapsed.
    void tick(uint32_t now_ms);

private:
    PollingTask tasks[MAX_POLLING_TASKS];
};

#endif // POLLING_ENGINE_H
