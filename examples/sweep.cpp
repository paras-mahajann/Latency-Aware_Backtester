#include <cstdint>
#include <iostream>

#include "feed_generator.hpp"
#include "simulator.hpp"

int main() {
  constexpr uint32_t kEpisodes = 5000;
  constexpr uint32_t kSeed = 42;
  const auto feed = generateDipFeed(kEpisodes, kSeed);

  std::cout << "total_latency_ns,position,pnl\n";

  for (uint64_t total = 0; total <= 7000; total += 500) {
    LatencyConfig cfg;
    cfg.feedLatencyNs = total / 4;
    cfg.computeLatencyNs = total / 4;
    cfg.orderLatencyNs = total - 2 * (total / 4); // remainder, so the parts always sum to `total`

    Simulator sim(cfg, EdgeTakerStrategy(5, 50));

    for (const auto &tm : feed)
      sim.addHistorical(tm.time, tm.msg);

    sim.run();

    const Portfolio &p = sim.portfolio();
    std::cout << total << "," << p.getPosition() << "," << p.markToMarket(110) << "\n";
  }
}