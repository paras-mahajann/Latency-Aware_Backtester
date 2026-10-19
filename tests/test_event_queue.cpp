#include "event_queue.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <vector>

TEST_CASE("EventQueue pops events in ascending timestamp order") {
  EventQueue q;

  q.push(SimEvent{500, OrderFields{1, 'B', 100, 10}});
  q.push(SimEvent{100, OrderFields{2, 'S', 105, 20}});
  q.push(SimEvent{300, OrderFields{3, 'B', 101, 30}});

  std::vector<uint64_t> popped;
  while (!q.empty()) {
    popped.push_back(q.top().timestamp);
    q.pop();
  }

  REQUIRE(popped.size() == 3);
  REQUIRE(popped[0] == 100);
  REQUIRE(popped[1] == 300);
  REQUIRE(popped[2] == 500);
}

TEST_CASE("EventQueue carries the right payload with each timestamp") {
  EventQueue q;

  q.push(SimEvent{200, OrderFields{7, 'S', 110, 5}});
  q.push(SimEvent{50, OrderFields{9, 'B', 99, 15}});

  // Earliest event should be the one with timestamp 50 and orderId 9
  const SimEvent &first = q.top();
  REQUIRE(first.timestamp == 50);

  const OrderFields *fields = std::get_if<OrderFields>(&first.payload);
  REQUIRE(fields != nullptr);
  REQUIRE(fields->orderId == 9);
  REQUIRE(fields->side == 'B');
  REQUIRE(fields->price == 99);
  REQUIRE(fields->shares == 15);
}

TEST_CASE("Empty EventQueue reports empty") {
  EventQueue q;
  REQUIRE(q.empty());

  q.push(SimEvent{1, OrderFields{1, 'B', 100, 1}});
  REQUIRE_FALSE(q.empty());
}

TEST_CASE("Events with equal timestamps pop in insertion order") {
  EventSchedular schedular;

  // Ten events, all at t=1000. Order IDs record the order they were pushed.
  for (uint64_t i = 1; i <= 10; i++) {
    schedular.schedule(1000, OrderFields{i, 'B', 100, 1});
  }

  std::vector<uint64_t> poppedIds;
  while (!schedular.empty()) {
    SimEvent event = schedular.popNext();
    const auto *f = std::get_if<OrderFields>(&event.payload);
    REQUIRE(f != nullptr);
    poppedIds.push_back(f->orderId);
  }

  std::vector<uint64_t> expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  REQUIRE(poppedIds == expected);
}