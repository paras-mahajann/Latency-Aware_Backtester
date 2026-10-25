#include <catch2/catch_test_macros.hpp>

#include "portfolio.hpp"

TEST_CASE("Round trip locks in profit") {
    Portfolio p;
    p.applyFill('B', 50, 100);
    p.applyFill('S', 55, 100);
    REQUIRE(p.getPosition() == 0);
    REQUIRE(p.getCash() == 500);
}

TEST_CASE("Open long position is marked to market") {
    Portfolio p;
    p.applyFill('B', 50, 100);
    REQUIRE(p.getPosition() == 100);
    REQUIRE(p.getCash() == -5000);
    REQUIRE(p.markToMarket(52) == 200);
}

TEST_CASE("Short position is marked to market") {
    Portfolio p;
    p.applyFill('S', 60, 50);
    REQUIRE(p.getPosition() == -50);
    REQUIRE(p.getCash() == 3000);
    REQUIRE(p.markToMarket(58) == 100);
}

TEST_CASE("Large notional does not overflow 32 bits") {
    Portfolio p;
    p.applyFill('B', 1500000,
                3000);  // $150.00 * 3000 shares = 4.5 billion in 1/10000 units
    REQUIRE(p.getCash() == -4500000000LL);
    REQUIRE(p.getPosition() == 3000);
}
