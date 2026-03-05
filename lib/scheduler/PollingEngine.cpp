#include "PollingEngine.h"

PollingEngine::PollingEngine() {
    for (size_t i = 0; i < MAX_POLLING_TASKS; ++i) {
        tasks[i].active = false;
        tasks[i].name = nullptr;
        tasks[i].interval_ms = 0;
        tasks[i].callback = nullptr;
        tasks[i].last_run_ms = 0;
    }
}

bool PollingEngine::addTask(const char* name, uint32_t interval_ms, PollingCallback callback) {
    if (callback == nullptr) return false;

    for (size_t i = 0; i < MAX_POLLING_TASKS; ++i) {
        if (!tasks[i].active) {
            tasks[i].name = name;
            tasks[i].interval_ms = interval_ms;
            tasks[i].callback = callback;
            tasks[i].last_run_ms = 0;
            tasks[i].active = true;
            return true;
        }
    }
    return false;
}

void PollingEngine::tick(uint32_t now_ms) {
    for (size_t i = 0; i < MAX_POLLING_TASKS; ++i) {
        if (tasks[i].active) {
            if ((now_ms - tasks[i].last_run_ms) >= tasks[i].interval_ms) {
                tasks[i].last_run_ms = now_ms;
                tasks[i].callback();
            }
        }
    }
}
