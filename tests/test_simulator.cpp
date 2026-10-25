#include "simulator.hpp"
#include <catch2/catch_test_macros.hpp>

static ITCHMessage makeAdd(uint64_t id, char side, uint32_t price,
                           uint32_t shares) {
  return AddOrderMessage{id, side, shares, {}, price};
}
static ITCHMessage makeDelete(uint64_t id) { return OrderDeleteMessage{id}; }

static Portfolio runScenario(LatencyConfig cfg) {
  Simulator sim(cfg, EdgeTakerStrategy(5, 50));
  sim.addHistorical(0, makeAdd(10, 'S', 110, 100));
  sim.addHistorical(0, makeAdd(11, 'B', 100, 100));
  sim.addHistorical(1000, makeAdd(20, 'S', 104, 50)); // the dip
  sim.addHistorical(2000, makeDelete(20));            // taken by someone faster
  sim.run();
  return sim.portfolio();
}

TEST_CASE("Zero latency: strategy captures the dip") {
  Portfolio p = runScenario(LatencyConfig{});
  REQUIRE(p.getPosition() == 50);
  REQUIRE(p.getCash() == -5200); // 50 shares at 104
}

TEST_CASE("Realistic latency: the dip is gone before the order arrives") {
  LatencyConfig cfg;
  cfg.feedLatencyNs = 500;
  cfg.computeLatencyNs = 100;
  cfg.orderLatencyNs = 2000; // order arrives at 3600
  Portfolio p = runScenario(cfg);
  REQUIRE(p.getPosition() == 0);
  REQUIRE(p.getCash() == 0);
}

TEST_CASE("Order arriving at 1999 still fills") {
  LatencyConfig cfg;
  cfg.feedLatencyNs = 500;
  cfg.orderLatencyNs = 499; // 1000 + 500 + 499
  REQUIRE(runScenario(cfg).getPosition() == 50);
}

TEST_CASE("Order arriving at exactly 2000 loses the tie to the exchange") {
  LatencyConfig cfg;
  cfg.feedLatencyNs = 500;
  cfg.orderLatencyNs = 500; // 1000 + 500 + 500
  REQUIRE(runScenario(cfg).getPosition() == 0);
}