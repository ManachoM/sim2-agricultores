
#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "event.h"

#include <functional>
#include <unordered_map>

class Simulation; // Forward declaration

using EventHandlerFunc = std::function<void(Simulation *, Event *)>;

class EventHandlerSystem {
private:
  // Map of agent_type -> process_type -> handler function
  std::unordered_map<int, std::unordered_map<int, EventHandlerFunc>> handlers;

  // Default handler for unregistered events
  EventHandlerFunc default_handler;

public:
  // Register a handler for a specific event type
  void
  register_handler(int agent_type, int process_type, EventHandlerFunc handler) {
    handlers[agent_type][process_type] = std::move(handler);
  }

  // Set the default handler
  void set_default_handler(EventHandlerFunc handler) {
    default_handler = std::move(handler);
  }

  // Handle an event
  void handle(Simulation *sim, Event *e) {
    int agent_type = e->get_type();
    int process_type = e->get_process();

    // Try to find a specific handler
    auto agent_it = handlers.find(agent_type);
    if (agent_it != handlers.end()) {
      auto process_it = agent_it->second.find(process_type);
      if (process_it != agent_it->second.end()) {
        // Found a specific handler
        process_it->second(sim, e);
        return;
      }
    }

    // If no specific handler found, use default
    if (default_handler) {
      default_handler(sim, e);
    }
  }
};

#endif // EVENT_HANDLER_H
