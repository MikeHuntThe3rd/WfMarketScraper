#pragma once
#include "VARS.hpp"
#include <string>
#include <unordered_map>

using namespace VARS;
class Trades {
public:
  unordered_map<string, Trade> Read();
  void Add(string id, Trade trd);
  void Remove(std::string id);

private:
  unordered_map<string, Trade> Trades_inner;
  std::mutex thread_lock_key;
  std::condition_variable cv;
};

inline Trades trades{};
