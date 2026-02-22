#include "Trades.hpp"
#include "VARS.hpp"
#include <algorithm>
#include <mutex>
#include <optional>
#include <vector>

void Trades::Add(Trade trade) {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  Trades_inner.push_back(trade);
  cv.notify_one();
}

void Trades::Remove(std::string id) {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  auto iter = this->FindById(id);
  if (iter.has_value()) {
    Trades_inner.erase(iter.value());
  }
  cv.notify_one();
}

void Trades::Edit(std::string id, Trade trade) {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  auto iter = this->FindById(id);
  if (iter.has_value()) {
    *iter.value() = trade;
  }
  cv.notify_one();
}

std::vector<VARS::Trade> Trades::Read() {
  std::lock_guard<std::mutex> lock(thread_lock_key);
  return Trades_inner;
}

std::optional<std::vector<VARS::Trade>::iterator>
Trades::FindById(std::string id) {
  auto find_iter =
      std::find_if(Trades_inner.begin(), Trades_inner.end(),
                   [&id](const Trade &trade) { return trade.id == id; });
  if (find_iter != Trades_inner.end()) {
    return find_iter;
  } else {
    return std::nullopt;
  }
}
