#include "Trades.hpp"
#include "VARS.hpp"
#include <mutex>

VARS::itemType GetITypeFromSlug(std::string slug) {
  for (const auto &curr : items["data"]) {
    if (curr["slug"] == slug) {
      std::vector<std::string> tags = curr["tags"];

      if (std::find(tags.begin(), tags.end(), "mod") != tags.end() &&
          std::find(tags.begin(), tags.end(), "veiled_riven") == tags.end()) {
        return VARS::itemType::mod;
      } else {
        return VARS::itemType::basic;
      }
    }
  }
  return VARS::itemType::Ayatan;
}

string GetSlugFromId(string id) {
  for (const json &curr : items["data"]) {
    if (curr["id"] == id)
      return curr["slug"];
  }
  return "0";
}

void Trds::Trades::Add(string id, Trade trd) {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  auto iter = Trades_inner.find(id);
  if (iter == Trades_inner.end())
    Trades_inner.insert({id, trd});
  cv.notify_one();
}

void Trds::Trades::Remove(std::string id) {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  auto iter = Trades_inner.find(id);
  if (iter != Trades_inner.end())
    Trades_inner.erase(iter);
  cv.notify_one();
}

void Trds::Trades::Set(string id, Trade trd) {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  auto iter = Trades_inner.find(id);
  if (iter != Trades_inner.end())
    iter->second = trd;
  cv.notify_one();
}

void Trds::Trades::Sync(json orders_my) {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  for (const json &order : orders_my["data"]) {
    if (Trades_inner.find(order["id"]) == Trades_inner.end()) {

      string slug = GetSlugFromId(order["itemId"]);
      VARS::itemType type = GetITypeFromSlug(slug);
      VARS::tradeType t_type = (order["type"] == "buy") ? VARS::tradeType::buy
                                                        : VARS::tradeType::sell;
      auto trd = Trade{slug,
                       order["itemId"],
                       order["platinum"],
                       order["platinum"],
                       type,
                       t_type,
                       true};

      switch (type) {
      case VARS::itemType::basic:
        Trades_inner.insert({order["id"], trd});
        break;
      case VARS::itemType::mod: {
        trd.level = order["rank"];
        Trades_inner.insert({order["id"], trd});
      }

      break;
      case VARS::itemType::Ayatan: {
        trd.amberStar = order["amberStars"];
        trd.cyanStar = order["cyanStars"];
        Trades_inner.insert({order["id"], trd});
        break;
      }
      default:
        break;
      }
    }
  }
  cv.notify_one();
}

unordered_map<string, Trade> Trds::Trades::Read() {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  return Trades_inner;
}
