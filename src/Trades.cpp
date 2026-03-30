#include "Trades.hpp"
#include "VARS.hpp"
#include <mutex>

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
      Trades_inner.insert({order["id"], Trade{}});
    }
  }
  cv.notify_one();
}

unordered_map<string, Trade> Trds::Trades::Read() {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  return Trades_inner;
}
