#pragma once
#include "CurlReq.hpp"
#include "OrderHandling.hpp"
#include "StringOps.hpp"
#include "VARS.hpp"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

using json = nlohmann::json;
using namespace std;
namespace Sorting {
// variables
extern std::string slug;
extern std::ofstream logfile;
// functions
bool Frequency(itemType type, std::optional<std::any> data = std::nullopt);
optional<Trade> ValidTrade(std::string item, std::vector<std::string> tags,
                           bool log = false);
std::optional<Trade> RankBasedMargin(json orders);
std::optional<Trade> AyatanMargin(json orders);
std::optional<Trade> BasicMargin(json orders);
} // namespace Sorting
