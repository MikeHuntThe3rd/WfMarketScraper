#pragma once
#include "CurlReq.hpp"
#include "StringOps.hpp"
#include "VARS.hpp"
#include "json.hpp"
#include <sstream>
#include <string>
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
json PostOrder(Trade trd);
json DeleteOrder(std::string id = "");
json UpdateOrder(string trade_id, Trade trd);

// trade manager functions
void HandleNewTrade(Trade trd);
void HandlelocalTrade(vector<string> item, tradeType trade_state);

} // namespace OrderHandling
