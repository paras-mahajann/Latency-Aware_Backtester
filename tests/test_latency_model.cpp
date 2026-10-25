#include <catch2/catch_test_macros.hpp>

#include "latency_model.hpp"

TEST_CASE("LatencyConfig applies feed, compute and order latency") {
    LatencyConfig cfg;
    cfg.feedLatencyNs = 500;
    cfg.computeLatencyNs = 100;
    cfg.orderLatencyNs = 2000;

    REQUIRE(cfg.strategySeesAt(1000) == 1500);
    REQUIRE(cfg.orderArrivesAt(1500) == 3600);
}

TEST_CASE("Default LatencyConfig is zero latency (naive backtest)") {
    LatencyConfig cfg;
    REQUIRE(cfg.strategySeesAt(1000) == 1000);
    REQUIRE(cfg.orderArrivesAt(1000) == 1000);
}