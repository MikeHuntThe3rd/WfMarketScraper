#pragma once
#include "CurlReq.hpp"
#include "OrderHandling.hpp"
#include "StringOps.hpp"
#include "VARS.hpp"
#include "json.hpp"
#include <optional>
#include <string>
#include <vector>

using json = nlohmann::json;
using namespace std;
namespace Sorting {
// variables
extern string slug;

// trade tracking
void SetBestTradePrice(VARS::Trade &trd);

// trade validation
bool Frequency(itemType type, optional<any> data = nullopt);
optional<Trade> ValidTrade(string item, vector<string> tags, bool log = false);

optional<Trade> RankBasedMargin(json orders);
optional<Trade> AyatanMargin(json orders);
optional<Trade> BasicMargin(json orders);
} // namespace Sorting
