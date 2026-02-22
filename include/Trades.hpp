#pragma once
#include "VARS.hpp"
#include <optional>
#include <string>
#include <vector>

using namespace VARS;

class Trades {
public:
  std::vector<VARS::Trade> Read();
  void Add(Trade trade);
  void Remove(std::string id);
  void Edit(std::string id, Trade trade);

private:
  std::optional<std::vector<VARS::Trade>::iterator> FindById(std::string id);
  std::vector<VARS::Trade> Trades_inner;
  std::mutex thread_lock_key;
  std::condition_variable cv;
};

inline Trades trades{};
