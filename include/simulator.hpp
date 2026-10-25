#pragma once
#include "apply_message.hpp"
#include "event_queue.hpp"
#include "latency_model.hpp"
#include "order_book.hpp"
#include "portfolio.hpp"
#include "strategy.hpp"
#include <cstdint>
#include <utility>
#include <variant>

class Simulator {
  LatencyConfig lat;
  EdgeTakerStrategy strategy;
  OrderBook trueBook;     // the exchange: updated at exchange time t
  OrderBook strategyBook; // the strategy's view: updated at t + feedLatency
  EventScheduler sched;
  Portfolio pf;

public:
  Simulator(LatencyConfig cfg, EdgeTakerStrategy s)
      : lat(cfg), strategy(std::move(s)) {}

  void addHistorical(uint64_t exchangeTime, ITCHMessage msg) {
    sched.schedule(exchangeTime, std::move(msg));
  }

  void run() {
    while (!sched.empty()) {
      SimEvent ev = sched.popNext(); // by value: we schedule new events below
      const uint64_t t = ev.timestamp;

      if (const auto *m = std::get_if<ITCHMessage>(&ev.payload)) {
        // Exchange applies the event now; the strategy hears about it later.
        applyMessage(trueBook, *m);
        sched.schedule(lat.strategySeesAt(t), FeedDelivery{*m});

      } else if (const auto *d = std::get_if<FeedDelivery>(&ev.payload)) {
        // Strategy's stale view catches up, then it may decide to trade.
        applyMessage(strategyBook, d->msg);
        if (auto order = strategy.onBookUpdate(strategyBook)) {
          sched.schedule(lat.orderArrivesAt(t), *order);
        }

      } else if (const auto *o = std::get_if<OrderFields>(&ev.payload)) {
        // Order reaches the exchange and trades against the book AS IT IS NOW.
        for (const Fill &f : trueBook.match(o->side, o->price, o->shares)) {
          pf.applyFill(o->side, f.price, f.shares);
        }
      }
    }
  }

  const Portfolio &portfolio() const { return pf; }
};