#include <catch2/catch_test_macros.hpp>

#include "feed_generator.hpp"

TEST_CASE("Same seed produces an identical feed") {
    auto a = generateDipFeed(50, 42);
    auto b = generateDipFeed(50, 42);
    REQUIRE(a.size() == b.size());
    for (size_t i = 0; i < a.size(); i++) {
        REQUIRE(a[i].time == b[i].time);
    }
}

TEST_CASE("Different seeds produce different dip lifetimes") {
    auto a = generateDipFeed(50, 1);
    auto b = generateDipFeed(50, 2);
    bool anyDifferent = false;
    for (size_t i = 0; i < a.size(); i++) {
        if (a[i].time != b[i].time) anyDifferent = true;
    }
    REQUIRE(anyDifferent);
}

TEST_CASE("Feed has four messages per episode") { REQUIRE(generateDipFeed(10, 7).size() == 40); }

TEST_CASE("Dip lifetimes stay within the configured range") {
    auto feed = generateDipFeed(200, 99);
    for (size_t e = 0; e < 200; e++) {
        const uint64_t dipAdd = feed[e * 4 + 1].time;
        const uint64_t dipDel = feed[e * 4 + 2].time;
        REQUIRE(dipDel - dipAdd >= 200);
        REQUIRE(dipDel - dipAdd <= 6000);
    }
}