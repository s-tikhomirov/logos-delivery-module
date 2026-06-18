// Shared test-observable state for the stubbed logos_events: methods.
// Lets tests check that start()/stop() emitted their completion events
// (nodeStarted / nodeStopped).
#pragma once

#include <cstdint>
#include <string>

namespace delivery_test_events {

struct NodeLifecycleEvent {
    bool success = false;
    std::string message;
    int64_t timestamp = 0;
    bool fired = false;  // set true once the event has been emitted at least once
};

struct StoreQueryEvent {
    bool success = false;
    std::string responseJson;
    int64_t timestamp = 0;
    bool fired = false;
};

extern NodeLifecycleEvent g_lastNodeStarted;
extern NodeLifecycleEvent g_lastNodeStopped;
extern StoreQueryEvent g_lastStoreQueryCompleted;

inline void resetNodeLifecycleEvents() {
    g_lastNodeStarted = NodeLifecycleEvent{};
    g_lastNodeStopped = NodeLifecycleEvent{};
}

inline void resetStoreQueryEvents() {
    g_lastStoreQueryCompleted = StoreQueryEvent{};
}

} // namespace delivery_test_events
