#pragma once
#include "itch_messages.hpp"
#include "order_book.hpp"
#include <type_traits>
#include <variant>

// Applies one parsed ITCH message to a book. Used for BOTH the true book
// and the strategy's delayed view, so they can never disagree on how a
// message is interpreted.
inline void applyMessage(OrderBook &book, const ITCHMessage &msg) {
  std::visit(
      [&book](const auto &m) {
        using T = std::decay_t<decltype(m)>;
        if constexpr (std::is_same_v<T, AddOrderMessage>) {
          book.addOrder(m.orderRefNum, m.side, m.price, m.shares);
        } else if constexpr (std::is_same_v<T, OrderExecutedMessage>) {
          book.executeOrder(m.orderRefNum, m.executedShares);
        } else if constexpr (std::is_same_v<T, OrderCancelMessage>) {
          book.cancelOrder(m.orderRefNum, m.cancelledShares);
        } else if constexpr (std::is_same_v<T, OrderDeleteMessage>) {
          book.deleteOrder(m.orderRefNum);
        } else {
          (void)m; // TradeMessage: tape print only, the book doesn't change
        }
      },
      msg);
}