#pragma once
#include "StringOps.hpp"
#include "VARS.hpp"
#include "json.hpp"
#include <string>
#include <unordered_map>

using namespace VARS;
using namespace std;
using namespace StringOps;
using json = nlohmann::json;

namespace Trds {

class Trades {
public:
  unordered_map<string, Trade> Read();
  void Add(string id, Trade trd);
  void Remove(string id);
  void Set(string id, Trade trd);
  void Sync(json orders_my);

private:
  unordered_map<string, Trade> Trades_inner;
  std::mutex thread_lock_key;
  std::condition_variable cv;
};

inline Trades trades{};
} // namespace Trds
