#pragma once
#include "event_queue.hpp"
#include "order_book.hpp"
#include <cstdint>
#include <optional>


class EdgeTakerStrategy {
  uint32_t triggerTicks;
  uint32_t orderShares;
  uint64_t nextOrderId = 1'000'000'000; 
  uint32_t lastBestAsk = 0;

public:
  EdgeTakerStrategy(uint32_t triggerTicks_, uint32_t orderShares_)
      : triggerTicks(triggerTicks_), orderShares(orderShares_) {}

  std::optional<OrderFields> onBookUpdate(const OrderBook &view) {
    const uint32_t ask = view.bestAsk();
    std::optional<OrderFields> order;

    // Addition form, not `lastBestAsk - ask >= triggerTicks`: these are
    // unsigned, so subtracting when the ask ROSE would wrap to a huge
    // number and fire a false trigger.
    if (ask != 0 && lastBestAsk != 0 && ask + triggerTicks <= lastBestAsk) {
      order = OrderFields{nextOrderId++, 'B', ask, orderShares};
    }

    lastBestAsk = ask; // always update, even when not triggering
    return order;
  }
};