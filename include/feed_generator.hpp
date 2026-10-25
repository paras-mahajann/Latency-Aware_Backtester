#pragma once
#include <cstdint>
#include <random>
#include <vector>

#include "itch_messages.hpp"

struct TimedMessage {
    uint64_t time;
    ITCHMessage msg;
};

inline std::vector<TimedMessage> generateDipFeed(uint32_t numEpisodes, uint32_t seed) {
    constexpr uint64_t kEpisodeSpacing = 100000;  // ns between episode starts
    constexpr uint64_t kDipDelay = 1000;          // ns from base to the dip
    constexpr uint64_t kClearDelay = 50000;       // ns from base to clearing the 110 ask
    constexpr uint64_t kMinDipLife = 200;         // ns
    constexpr uint64_t kMaxDipLife = 6000;        // ns

    std::mt19937 rng(seed);
    std::uniform_int_distribution<uint64_t> dipLife(kMinDipLife, kMaxDipLife);

    std::vector<TimedMessage> feed;
    feed.reserve(static_cast<size_t>(numEpisodes) * 4);

    for (uint32_t e = 0; e < numEpisodes; e++) {
        const uint64_t base = static_cast<uint64_t>(e) * kEpisodeSpacing;
        const uint64_t normalId = static_cast<uint64_t>(e) * 10 + 1;
        const uint64_t dipId = static_cast<uint64_t>(e) * 10 + 2;
        const uint64_t life = dipLife(rng);

        // AddOrderMessage fields: orderRefNum, side, shares, symbol, price
        feed.push_back({base, AddOrderMessage{normalId, 'S', 100, {}, 110}});
        feed.push_back({base + kDipDelay, AddOrderMessage{dipId, 'S', 50, {}, 104}});
        feed.push_back({base + kDipDelay + life, OrderDeleteMessage{dipId}});
        feed.push_back({base + kClearDelay, OrderDeleteMessage{normalId}});
    }

    return feed;
}