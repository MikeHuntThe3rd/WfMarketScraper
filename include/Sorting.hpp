#pragma once
#include "CurlReq.hpp"
#include "OrderHandling.hpp"
#include "VARS.hpp"
#include "json.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

using json = nlohmann::json;
using namespace VARS;
namespace Sorting {
// variables
extern std::string slug;
extern std::ofstream logfile;
// functions
bool Frequency(itemType type, std::optional<std::any> data = std::nullopt);
Trade ValidTrade(std::string item, std::vector<std::string> tags,
                 bool log = false);
std::optional<rank> RankBasedMargin(json orders);
std::optional<ayatan_sculpture> AyatanMargin(json orders);
std::optional<basic> BasicMargin(json orders);
} // namespace Sorting
