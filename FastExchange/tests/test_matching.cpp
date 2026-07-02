#include "matching/price_time_fifo.hpp"
#include "orderbook/order_book.hpp"
#include <gtest/gtest.h>

TEST(MatchingTest, PartialFill) {
    fastexchange::OrderBook book;
    fastexchange::PriceTimeFifoMatcher matcher;

    fastexchange::Order sell{1, "ABC", fastexchange::Side::Sell, fastexchange::OrderType::Limit,
                             150, 50, 50, 0, 0};
    book.add_order(sell);

    fastexchange::Order buy{2, "ABC", fastexchange::Side::Buy, fastexchange::OrderType::Limit, 150,
                            100, 100, 0, 0};
    auto result = matcher.match(book, buy);

    ASSERT_EQ(result.trades.size(), 1);
    EXPECT_EQ(result.trades[0].quantity, 50);
    EXPECT_TRUE(result.remainder.has_value());
    EXPECT_EQ(result.remainder->remaining, 50);

    book.add_order(*result.remainder);
    auto snap = book.snapshot();
    EXPECT_EQ(snap.best_bid, 150);
    EXPECT_EQ(snap.bid_depth, 50);
}

TEST(MatchingTest, MultiSymbolIsolation) {
    fastexchange::OrderBook book_abc;
    fastexchange::OrderBook book_xyz;
    fastexchange::PriceTimeFifoMatcher matcher;

    fastexchange::Order sell_abc{1, "ABC", fastexchange::Side::Sell, fastexchange::OrderType::Limit,
                                 100, 10, 10, 0, 0};
    book_abc.add_order(sell_abc);

    fastexchange::Order buy_xyz{2, "XYZ", fastexchange::Side::Buy, fastexchange::OrderType::Limit,
                                100, 10, 10, 0, 0};
    auto result = matcher.match(book_xyz, buy_xyz);
    EXPECT_TRUE(result.trades.empty());
    EXPECT_EQ(book_abc.trades().size(), 0);
}
