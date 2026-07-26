#pragma once

// Holds "has a result arrived yet, and what was it" state for one pending
// request. Generic - knows nothing about login/register/messages, just
// wraps a caller-supplied result type.
template<typename TResult>
struct PendingResult {
    bool received = false;
    TResult value{};

    void reset() {
        received = false;
        value = TResult{};
    }

    void set(const TResult& v) {
        received = true;
        value = v;
    }
};
