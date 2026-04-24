#pragma once
#include "CurlReq.hpp"
#include "StringOps.hpp"
#include "VARS.hpp"
#include "json.hpp"
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace VARS;
using namespace StringOps;
using namespace CurlReq;
using namespace std;
using json = nlohmann::json;

namespace OrderHandling {

// Major functions(should be in its own thread)
void EElogChecking();
// Helper functions
void HandlelocalTrade(vector<string> item, tradeType trade_state);
vector<pair<string, VARS::tradeType>>
ParseLocalTrades(vector<vector<string>> soldItems,
                 vector<vector<string>> boughtItems);

// web operations
json PostOrder(Trade trd);
json DeleteOrder(std::string id = "");
json UpdateOrder(string trade_id, Trade trd);

// web trade manager functions
json FindItem(string id);
void ManageOneTrade(json order);
void HandleNewTrade(Trade trd);
void HandleAllTrades();
} // namespace OrderHandling
