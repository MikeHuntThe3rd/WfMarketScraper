#include "Trades.hpp"
#include "VARS.hpp"
#include <mutex>

void Trades::Add(string id, Trade trd) {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  auto iter = Trades_inner.find(id);
  if (iter == Trades_inner.end())
    Trades_inner.insert({id, trd});
  cv.notify_one();
}

void Trades::Remove(std::string id) {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  auto iter = Trades_inner.find(id);
  if (iter != Trades_inner.end())
    Trades_inner.erase(iter);
  cv.notify_one();
}

unordered_map<string, Trade> Trades::Read() {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  return Trades_inner;
}
