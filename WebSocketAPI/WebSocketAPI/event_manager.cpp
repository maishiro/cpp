#include "event_manager.h"

EventManager::EventManager() {
}

void EventManager::publish_event(const Event& event) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    event_queue_.push(event);
    log_info("=== Event published: type=" + event.type + ", queue_size=" + std::to_string(event_queue_.size()) + " ===");
}

bool EventManager::get_next_event(Event& out_event) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (event_queue_.empty()) {
        return false;
    }
    out_event = event_queue_.front();
    event_queue_.pop();
    log_info("=== Event dequeued: type=" + out_event.type + " ===");
    return true;
}

bool EventManager::has_events() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return !event_queue_.empty();
}

void EventManager::clear_events() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!event_queue_.empty()) {
        event_queue_.pop();
    }
}

// Global event manager instance
EventManager& get_event_manager() {
    static EventManager instance;
    return instance;
}
