#include "order_book.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Buy sweeps asks in price order and fills at resting price") {
  OrderBook book;
  book.addOrder(1, 'S', 103, 60);
  book.addOrder(2, 'S', 104, 50);
  book.addOrder(3, 'S', 106, 40);

  auto fills = book.match('B', 105, 100);

  REQUIRE(fills.size() == 2);
  REQUIRE(fills[0].restingOrderId == 1);
  REQUIRE(fills[0].price == 103);
  REQUIRE(fills[0].shares == 60);
  REQUIRE(fills[1].restingOrderId == 2);
  REQUIRE(fills[1].price == 104);
  REQUIRE(fills[1].shares == 40);
  REQUIRE(book.getOrderShares(2) == 10);
  REQUIRE(book.bestAsk() == 104);
}

TEST_CASE("Order that doesn't cross the spread gets no fills") {
  OrderBook book;
  book.addOrder(1, 'S', 105, 50);

  REQUIRE(book.match('B', 104, 50).empty());
  REQUIRE(book.bestAsk() == 105);
}

TEST_CASE("Fills respect FIFO within a price level") {
  OrderBook book;
  book.addOrder(1, 'S', 100, 30);
  book.addOrder(2, 'S', 100, 30);

  auto fills = book.match('B', 100, 40);

  REQUIRE(fills.size() == 2);
  REQUIRE(fills[0].restingOrderId == 1);
  REQUIRE(fills[0].shares == 30);
  REQUIRE(fills[1].restingOrderId == 2);
  REQUIRE(fills[1].shares == 10);
}

TEST_CASE("Unfilled remainder is discarded, not rested") {
  OrderBook book;
  book.addOrder(1, 'S', 100, 50);

  auto fills = book.match('B', 100, 200);

  REQUIRE(fills.size() == 1);
  REQUIRE(fills[0].shares == 50);
  REQUIRE(book.bestBid() == 0);
}

TEST_CASE(
    "Sell sweeps bids from highest price down and fills at resting price") {
  OrderBook book;
  book.addOrder(1, 'B', 100, 60);
  book.addOrder(2, 'B', 99, 50);
  book.addOrder(3, 'B', 97, 40);

  auto fills = book.match('S', 98, 100);

  REQUIRE(fills.size() == 2);
  REQUIRE(fills[0].restingOrderId == 1);
  REQUIRE(fills[0].price == 100);
  REQUIRE(fills[0].shares == 60);
  REQUIRE(fills[1].restingOrderId == 2);
  REQUIRE(fills[1].price == 99);
  REQUIRE(fills[1].shares == 40);
  REQUIRE(book.getOrderShares(2) == 10);
  REQUIRE(book.bestBid() == 99);
}

TEST_CASE("Sell that doesn't cross the spread gets no fills") {
  OrderBook book;
  book.addOrder(1, 'B', 100, 50);

  REQUIRE(book.match('S', 101, 50).empty());
  REQUIRE(book.bestBid() == 100);
}
TEST_CASE("Executing more shares than remain removes the order, no underflow") {
  OrderBook book;
  book.addOrder(1, 'S', 100, 50);
  book.executeOrder(1, 80);

  REQUIRE(book.getOrderShares(1) == 0);
  REQUIRE(book.bestAsk() == 0);
}
TEST_CASE("Partial cancel leaves the order resting with fewer shares") {
  OrderBook book;
  book.addOrder(1, 'B', 50, 100);
  book.cancelOrder(1, 30);

  REQUIRE(book.bestBid() == 50);
  REQUIRE(book.getOrderShares(1) == 70);
}

TEST_CASE("Cancelling more shares than remain removes the order, no underflow") {
  OrderBook book;
  book.addOrder(1, 'S', 100, 50);
  book.cancelOrder(1, 80);

  REQUIRE(book.getOrderShares(1) == 0);
  REQUIRE(book.bestAsk() == 0);
}

TEST_CASE("addOrder fails cleanly when the pool is exhausted") {
  OrderBook book;
  bool allAdded = true;
  for (uint64_t i = 1; i <= 100000; i++) {
    allAdded = allAdded && book.addOrder(i, 'S', 200, 1);
  }
  REQUIRE(allAdded);

  REQUIRE_FALSE(book.addOrder(100001, 'B', 150, 1));
  REQUIRE(book.bestBid() == 0); // no phantom empty price level
  REQUIRE(book.getOrderShares(100001) == 0);

  book.deleteOrder(1);                         // frees one slot
  REQUIRE(book.addOrder(100001, 'B', 150, 1)); // and the pool recovers
  REQUIRE(book.bestBid() == 150);
}

TEST_CASE("addOrder rejects a duplicate order ID without corrupting the book") {
  OrderBook book;
  REQUIRE(book.addOrder(1, 'S', 100, 10));
  REQUIRE_FALSE(book.addOrder(1, 'S', 101, 5));
  REQUIRE(book.getOrderShares(1) == 10);
  REQUIRE(book.bestAsk() == 100);
}
