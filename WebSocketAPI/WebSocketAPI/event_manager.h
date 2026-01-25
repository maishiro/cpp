#pragma once

#include "common.h"
#include <queue>
#include <mutex>

/**
 * Thread-safe event queue and broadcast manager
 */
class EventManager {
public:
    EventManager();
    ~EventManager() = default;

    // Deleted copy/move operations
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    /**
     * Enqueue an event for broadcasting
     */
    void publish_event(const Event& event);

    /**
     * Get and remove the next event from queue
     */
    bool get_next_event(Event& out_event);

    /**
     * Check if there are pending events
     */
    bool has_events() const;

    /**
     * Clear all pending events
     */
    void clear_events();

private:
    std::queue<Event> event_queue_;
    mutable std::mutex queue_mutex_;
};

/**
 * Get global event manager instance
 */
EventManager& get_event_manager();
