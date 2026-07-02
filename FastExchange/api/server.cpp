#include "server.hpp"
#include <crow.h>
#include <fstream>
#include <iostream>

namespace fastexchange {

void ApiState::load_metrics_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("cannot open metrics file: " + path);
    }
    in >> metrics_cache;
    has_metrics_cache = true;
    if (metrics_cache.contains("scenario")) {
        scenario_name = metrics_cache["scenario"].get<std::string>();
    }
}

void ApiState::register_symbols(const std::vector<std::string>& symbols) {
    if (!registry) registry = std::make_unique<SymbolRegistry>();
    for (const auto& sym : symbols) {
        if (!registry->has_symbol(sym)) {
            registry->register_symbol(sym);
        }
    }
}

void ApiState::sync_live_snapshot(const nlohmann::json& snapshot,
                                 const std::vector<Trade>& trades) {
    std::lock_guard lock(mutex);
    metrics_cache = snapshot;
    has_metrics_cache = true;
    recent_trades = trades;
    if (metrics_cache.contains("scenario")) {
        scenario_name = metrics_cache["scenario"].get<std::string>();
    }
}

ApiServer::ApiServer(ApiState& state) : state_(state) {}

void ApiServer::run(const std::string& host, int port) {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/metrics")
    ([this]() {
        std::lock_guard lock(state_.mutex);
        if (state_.has_metrics_cache) {
            return crow::response{state_.metrics_cache.dump()};
        }
        return crow::response{state_.metrics.snapshot().dump()};
    });

    CROW_ROUTE(app, "/scenario")
    ([this]() {
        std::lock_guard lock(state_.mutex);
        nlohmann::json j;
        j["name"] = state_.scenario_name;
        j["running"] = state_.running.load();
        return crow::response{j.dump()};
    });

    CROW_ROUTE(app, "/book/<string>")
    ([this](const std::string& symbol) {
        std::lock_guard lock(state_.mutex);
        nlohmann::json j;
        if (!state_.registry || !state_.registry->has_symbol(symbol)) {
            return crow::response{404, "unknown symbol"};
        }
        auto snap = state_.registry->get_book(symbol).snapshot();
        j["symbol"] = symbol;
        j["best_bid"] = snap.best_bid;
        j["best_ask"] = snap.best_ask;
        j["bid_depth"] = snap.bid_depth;
        j["ask_depth"] = snap.ask_depth;

        nlohmann::json bids = nlohmann::json::array();
        for (const auto& [price, dq] : state_.registry->get_book(symbol).bids()) {
            Quantity total = 0;
            for (const auto& o : dq) total += o.remaining;
            bids.push_back({{"price", price}, {"quantity", total}});
        }
        nlohmann::json asks = nlohmann::json::array();
        for (const auto& [price, dq] : state_.registry->get_book(symbol).asks()) {
            Quantity total = 0;
            for (const auto& o : dq) total += o.remaining;
            asks.push_back({{"price", price}, {"quantity", total}});
        }
        j["bids"] = bids;
        j["asks"] = asks;
        return crow::response{j.dump()};
    });

    CROW_ROUTE(app, "/trades")
    ([this]() {
        std::lock_guard lock(state_.mutex);
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& t : state_.recent_trades) {
            arr.push_back({{"symbol", t.symbol},
                           {"price", t.price},
                           {"quantity", t.quantity},
                           {"buy_order_id", t.buy_order_id},
                           {"sell_order_id", t.sell_order_id},
                           {"timestamp", t.timestamp}});
        }
        return crow::response{arr.dump()};
    });

    CROW_ROUTE(app, "/health")
    ([]() { return crow::response{"{\"status\":\"ok\"}"}; });

    std::cout << "FastExchange API listening on http://" << host << ":" << port << "\n";
    app.bindaddr(host).port(port).multithreaded().run();
}

}  // namespace fastexchange
