#pragma once
#include<cstdint>
#include<stdexcept>
#include<utility>
#include<queue>
#include<vector>
#include<variant>
#include "itch_messages.hpp"

struct OrderFields {
  uint64_t orderId;
  char side;
  uint32_t price;
  uint32_t shares;
};

struct FeedDelivery {
  ITCHMessage msg;
};

using EventPayload = std::variant<ITCHMessage, OrderFields, FeedDelivery>;


struct SimEvent {
  uint64_t timestamp;
  EventPayload payload;
  uint64_t sequence = 0;
};

struct EventComparator {
  bool operator()(const SimEvent &a, const SimEvent &b) const{
    if (a.timestamp != b.timestamp)
      return a.timestamp > b.timestamp;
    return a.sequence > b.sequence;
  }
};

using EventQueue =
    std::priority_queue<SimEvent, std::vector<SimEvent>, EventComparator>;

class EventScheduler {
  EventQueue queue;
  uint64_t nextSequence = 0;

public:
    void schedule(uint64_t timestamp,EventPayload payload) 
    {
        queue.push(SimEvent{timestamp, std::move(payload), nextSequence++});
    }

    bool empty() const noexcept 
    { 
        return queue.empty();
    }

    SimEvent popNext() { 
        if (queue.empty()) {
            throw std::out_of_range("EventScheduler is empty");
        }
        SimEvent e = queue.top();
        queue.pop();
        return e;
    }
};

