#include <benchmark/benchmark.h>

#include <cstdint>
#include <memory>

#include "event_queue.hpp"
#include "feed_generator.hpp"
#include "order_book.hpp"
#include "simulator.hpp"

// NOTE: no BENCHMARK_MAIN() here. benchmarks/CMakeLists.txt links
// benchmark::benchmark_main, which already provides main().

// 1) Scheduler throughput: push N events with pseudo-random timestamps, then
//    pop them all in order. With N = 100000 and 24-bit timestamps there are
//    many equal timestamps, so the sequence tie-break is exercised too.
//    Items = events pushed (each is also popped once).
static void BM_SchedulerPushPop(benchmark::State& state) {
    const auto n = static_cast<uint64_t>(state.range(0));
    for (auto _ : state) {
        EventScheduler sched;
        uint64_t x = 12345;
        for (uint64_t i = 0; i < n; i++) {
            x = x * 6364136223846793005ULL + 1442695040888963407ULL;  // LCG, deterministic
            sched.schedule(x >> 40, OrderFields{i, 'B', 100, 1});
        }
        uint64_t checksum = 0;
        while (!sched.empty()) {
            checksum += sched.popNext().timestamp;
        }
        benchmark::DoNotOptimize(checksum);
    }
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(n));
}
BENCHMARK(BM_SchedulerPushPop)->Arg(1000)->Arg(100000);

// 2) Matching cost for one single-share fill against a resting order. The
//    resting order is large enough that it is never exhausted during the run,
//    so each iteration measures: level lookup + fill + executeOrder + the
//    returned vector.
static void BM_MatchSingleFill(benchmark::State& state) {
    OrderBook book;
    book.addOrder(1, 'S', 100, 4'000'000'000u);
    for (auto _ : state) {
        auto fills = book.match('B', 100, 1);
        benchmark::DoNotOptimize(fills);
    }
}
BENCHMARK(BM_MatchSingleFill);

// 3) End-to-end simulation: replay 1,000 dip episodes (4,000 historical
//    messages) through the whole pipeline at a given total latency. Simulator
//    construction and teardown (two order books, each with a 100,000-slot
//    pool) are excluded from the timing. Items = historical messages replayed.
static void BM_SimulatorRun(benchmark::State& state) {
    static const auto feed = generateDipFeed(1000, 42);

    const auto total = static_cast<uint64_t>(state.range(0));
    LatencyConfig cfg;
    cfg.feedLatencyNs = total / 4;
    cfg.computeLatencyNs = total / 4;
    cfg.orderLatencyNs = total - 2 * (total / 4);

    for (auto _ : state) {
        state.PauseTiming();
        auto sim = std::make_unique<Simulator>(cfg, EdgeTakerStrategy(5, 50));
        state.ResumeTiming();

        for (const auto& tm : feed) {
            sim->addHistorical(tm.time, tm.msg);
        }
        sim->run();
        benchmark::DoNotOptimize(sim->portfolio().getPosition());

        state.PauseTiming();
        sim.reset();
        state.ResumeTiming();
    }
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(feed.size()));
}
BENCHMARK(BM_SimulatorRun)->Arg(0)->Arg(3000)->Arg(6000);