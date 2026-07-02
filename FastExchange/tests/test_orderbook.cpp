#include "orderbook/order_book.hpp"
#include <gtest/gtest.h>

TEST(OrderBookTest, AddAndCancel) {
    fastexchange::OrderBook book;
    fastexchange::Order o;
    o.id = 1;
    o.symbol = "ABC";
    o.side = fastexchange::Side::Buy;
    o.price = 150;
    o.quantity = 100;
    o.remaining = 100;

    EXPECT_TRUE(book.add_order(o));
    auto snap = book.snapshot();
    EXPECT_EQ(snap.best_bid, 150);
    EXPECT_EQ(snap.bid_depth, 100);

    EXPECT_TRUE(book.cancel_order(1));
    snap = book.snapshot();
    EXPECT_EQ(snap.bid_depth, 0);
}

TEST(OrderBookTest, ModifyOrder) {
    fastexchange::OrderBook book;
    fastexchange::Order o{1, "ABC", fastexchange::Side::Sell, fastexchange::OrderType::Limit,
                          200, 50, 50, 0, 0};
    book.add_order(o);
    EXPECT_TRUE(book.modify_order(1, 199, 30));
    auto snap = book.snapshot();
    EXPECT_EQ(snap.best_ask, 199);
    EXPECT_EQ(snap.ask_depth, 30);
}
