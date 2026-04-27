#pragma once
#include "CurlReq.hpp"
#include "StringOps.hpp"
#include "VARS.hpp"
#include "json.hpp"
#include <string>
#include <vector>

using namespace VARS;
using namespace StringOps;
using namespace CurlReq;
using namespace std;
using json = nlohmann::json;

namespace OrderHandling {
// types
struct local_trade {
  string id;
  int amount;
  VARS::tradeType type;

  local_trade(string id, VARS::tradeType type, int amount)
      : id(id), type(type), amount(amount) {}
};

// Major functions(should be in its own thread)
void EElogChecking();
// Helper functions
vector<local_trade> ParseLocalTrades(vector<vector<string>> soldItems,
                                     vector<vector<string>> boughtItems,
                                     json orders);
local_trade ParseSingleTrade(vector<string> data, VARS::tradeType type);
void HandlelocalTrade(local_trade trd);
optional<local_trade> HandleSet(vector<local_trade> &trades,
                                local_trade current, json orders);

// web operations
bool PostOrder(Trade trd);
bool DeleteOrder(std::string id = "");
bool UpdateOrder(string trade_id, Trade trd);

// web trade manager functions
json FindItem(string id);
void ManageOneTrade(json order);
void HandleNewTrade(Trade trd);
void HandleAllTrades();
} // namespace OrderHandling
