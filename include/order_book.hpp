#pragma once
#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>

#include "pool_allocator.hpp"

struct Order {
    uint64_t orderId;
    uint32_t price;
    uint32_t shares;
    char side;
    Order(uint64_t id, uint32_t p, uint32_t s, char sd)
        : orderId(id), price(p), shares(s), side(sd) {}
};

struct OrderLocation {
    uint32_t price;
    char side;
    std::list<Order*>::iterator listIt;
};

struct Fill {
    uint64_t restingOrderId;
    uint32_t price;
    uint32_t shares;
};

class OrderBook {
    std::map<uint32_t, std::list<Order*>, std::greater<uint32_t>> bids;
    std::map<uint32_t, std::list<Order*>> asks;
    std::unordered_map<uint64_t, OrderLocation> orderIndex;
    PoolAllocator<Order, 100000> pool;

   private:
    template <typename Book>
    void matchAgainst(Book& book, bool isBuy, uint32_t limitPrice, uint32_t& remaining,
                      std::vector<Fill>& fills) {
        while (remaining > 0 && !book.empty()) {
            auto levelIt = book.begin();  // re-fetched every iteration
            const uint32_t levelPrice = levelIt->first;

            const bool crosses = isBuy ? (levelPrice <= limitPrice) : (levelPrice >= limitPrice);
            if (!crosses) break;

            Order* resting = levelIt->second.front();  // FIFO: oldest order at best price
            const uint32_t fillQty = std::min(remaining, resting->shares);

            fills.push_back(
                Fill{resting->orderId, levelPrice, fillQty});  // record BEFORE executeOrder
            remaining -= fillQty;

            executeOrder(resting->orderId,
                         fillQty);  // may free `resting` and erase the level
        }
    }

    std::list<Order*>& getOrCreatePriceLevel(char side, uint32_t price) {
        if (side == 'B') {
            return bids[price];
        } else {
            return asks[price];
        }
    }

    void erasePriceLevelIfEmpty(char side, uint32_t price, const std::list<Order*>& priceLevel) {
        if (!priceLevel.empty()) return;
        if (side == 'B') {
            bids.erase(price);
        } else {
            asks.erase(price);
        }
    }

   public:
    std::vector<Fill> match(char side, uint32_t limitPrice, uint32_t shares) {
        std::vector<Fill> fills;
        uint32_t remaining = shares;
        if (side == 'B') {
            matchAgainst(asks, true, limitPrice, remaining, fills);
        } else {
            matchAgainst(bids, false, limitPrice, remaining, fills);
        }
        return fills;
    }

    std::list<uint64_t> getOrdersAtPrice(char side, uint32_t price) const {
        std::list<uint64_t> result;
        if (side == 'B') {
            auto it = bids.find(price);
            if (it == bids.end()) return result;
            for (Order* order : it->second) result.push_back(order->orderId);
        } else {
            auto it = asks.find(price);
            if (it == asks.end()) return result;
            for (Order* order : it->second) result.push_back(order->orderId);
        }
        return result;
    }
    uint32_t getOrderShares(uint64_t orderId) const {
        auto idxIt = orderIndex.find(orderId);
        if (idxIt == orderIndex.end()) return 0;
        Order* order = *idxIt->second.listIt;
        return order->shares;
    }

    uint32_t bestBid() const {
        if (bids.empty()) return 0;
        return bids.begin()->first;
    }
    uint32_t bestAsk() const {
        if (asks.empty()) return 0;
        return asks.begin()->first;
    }
    bool addOrder(uint64_t orderId, char side, uint32_t price, uint32_t shares) {
        if (orderIndex.find(orderId) != orderIndex.end()) return false;  // duplicate ID

        Order* order = pool.construct(orderId, price, shares, side);
        if (order == nullptr) return false;  // pool exhausted

        auto& priceLevel = getOrCreatePriceLevel(side, price);
        priceLevel.push_back(order);
        orderIndex[orderId] = OrderLocation{price, side, std::prev(priceLevel.end())};
        return true;
    }

    void cancelOrder(uint64_t orderId, uint32_t sharesToCancel) {
        auto idxIt = orderIndex.find(orderId);
        if (idxIt == orderIndex.end()) return;

        OrderLocation& loc = idxIt->second;
        const char side = loc.side;
        const uint32_t price = loc.price;
        auto& priceLevel = getOrCreatePriceLevel(side, price);

        Order* order = *loc.listIt;
        order->shares -= std::min(sharesToCancel, order->shares);

        if (order->shares == 0) {
            priceLevel.erase(loc.listIt);
            orderIndex.erase(idxIt);
            pool.destroy(order);
            erasePriceLevelIfEmpty(side, price, priceLevel);
        }
    }

    void executeOrder(uint64_t orderId, uint32_t sharesExecuted) {
        auto idxIt = orderIndex.find(orderId);
        if (idxIt == orderIndex.end()) return;

        OrderLocation& loc = idxIt->second;
        const char side = loc.side;
        const uint32_t price = loc.price;
        auto& priceLevel = getOrCreatePriceLevel(side, price);

        Order* order = *loc.listIt;
        const uint32_t reduceBy = std::min(sharesExecuted, order->shares);
        order->shares -= reduceBy;

        if (order->shares == 0) {
            priceLevel.erase(loc.listIt);
            orderIndex.erase(idxIt);
            pool.destroy(order);
            erasePriceLevelIfEmpty(side, price, priceLevel);
        }
    }

    void deleteOrder(uint64_t orderId) {
        auto idxIt = orderIndex.find(orderId);
        if (idxIt == orderIndex.end()) return;

        OrderLocation& loc = idxIt->second;
        const char side = loc.side;
        const uint32_t price = loc.price;
        auto& priceLevel = getOrCreatePriceLevel(side, price);

        Order* order = *loc.listIt;

        priceLevel.erase(loc.listIt);
        orderIndex.erase(idxIt);
        pool.destroy(order);
        erasePriceLevelIfEmpty(side, price, priceLevel);
    }
};