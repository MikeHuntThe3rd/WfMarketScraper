#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <optional>
#include "json.hpp"
#include "VARS.hpp"
#include "CurlReq.hpp"

using json = nlohmann::json;
using namespace VARS;
namespace Sorting {
    //variables
    extern std::string slug;
    extern std::ofstream logfile;
    //functions
    bool Frequency(itemType type, std::optional<std::any> data = std::nullopt);
    trade_return ValidTrade(std::string item, std::vector<std::string> tags, bool log = false);
    std::optional<rank> RankBasedMargin(json orders);
    std::optional<ayatan_sculpture> AyatanMargin(json orders);
    std::optional<basic> BasicMargin(json orders);
}