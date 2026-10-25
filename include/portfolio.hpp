#pragma once
#include <cstdint>

class Portfolio {
    int64_t cash = 0;
    int64_t position = 0;

   public:
    void applyFill(char side, uint32_t price, uint32_t shares) {
        const int64_t notional = static_cast<int64_t>(price) * shares;
        if (side == 'B') {
            cash -= notional;
            position += static_cast<int64_t>(shares);
        } else {
            cash += notional;
            position -= static_cast<int64_t>(shares);
        }
    }
    int64_t getPosition() const { return position; }
    int64_t getCash() const { return cash; }
    int64_t markToMarket(uint32_t markPrice) const {
        // cash + position * markPrice
        return cash + position * static_cast<int64_t>(markPrice);
    }
};