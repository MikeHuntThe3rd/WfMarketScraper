#include "Trades.hpp"
#include "StringOps.hpp"
#include "VARS.hpp"
#include <mutex>
#include <optional>

namespace Trds {
void Trades::Add(string id, Trade trd) {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  auto iter = Trades_inner.find(id);
  if (iter == Trades_inner.end())
    Trades_inner.insert({id, trd});
}

void Trades::Remove(std::string id) {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  auto iter = Trades_inner.find(id);
  if (iter != Trades_inner.end())
    Trades_inner.erase(iter);
}

void Trades::Set(string id, Trade trd) {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  auto iter = Trades_inner.find(id);
  if (iter != Trades_inner.end())
    iter->second = trd;
}

optional<json> Trades::FindTrade(string trade_id, json remote_trades) {
  for (const json &trade : remote_trades["data"]) {
    if (trade["id"] == trade_id) {
      return trade;
    }
  }
  return nullopt;
}

void Trades::Sync(json orders_my) {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  // create and update
  for (const json &order : orders_my["data"]) {
    auto iter = Trades_inner.find(order["id"]);
    if (iter == Trades_inner.end()) {

      string slug = GetSlugFromId(order["itemId"]);
      VARS::itemType i_type = GetITypeFromSlug(slug);
      VARS::tradeType t_type = (order["type"] == "buy") ? VARS::tradeType::buy
                                                        : VARS::tradeType::sell;
      auto trd = Trade{slug,
                       order["itemId"],
                       order["platinum"],
                       order["platinum"],
                       i_type,
                       t_type,
                       true};

      switch (i_type) {
      case VARS::itemType::mod: {
        trd.level = order["rank"];
      }

      break;
      case VARS::itemType::Ayatan: {
        trd.amberStar = order["amberStars"];
        trd.cyanStar = order["cyanStars"];
        break;
      }
      default:
        break;
      }
      Trades_inner.insert({order["id"], trd});
    } else {
      switch (iter->second.Tstate) {
      case VARS::tradeType::buy:
        iter->second.buy = order["platinum"];
        break;
      case VARS::tradeType::sell:
        iter->second.sell = order["platinum"];
        break;
      }
    }
  }

  // deleting
  bool can_delete;
  for (auto &local_trade : Trades_inner) {
    can_delete = true;
    for (const json &remote_trade : orders_my["data"]) {
      if (local_trade.first == remote_trade["id"]) {
        can_delete = false;
        break;
      }
    }
    if (can_delete)
      Trades_inner.erase(local_trade.first);
  }
}

unordered_map<string, Trade> Trds::Trades::Read() {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  return Trades_inner;
}
} // namespace Trds
