#pragma once
#include <cstdint>

struct LatencyConfig {
  uint64_t feedLatencyNs = 0;
  uint64_t computeLatencyNs = 0;
  uint64_t orderLatencyNs = 0;

  uint64_t strategySeesAt(uint64_t exchangeTime) const {
    return feedLatencyNs + exchangeTime;
  }
  uint64_t orderArrivesAt(uint64_t strategySawAt) const {
    return computeLatencyNs + orderLatencyNs + strategySawAt;
  }
};

