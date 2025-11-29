#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <limits>
#include <optional>
#include <algorithm>
#include "json.hpp"
#include "CurlReq.hpp"

using json = nlohmann::json;
namespace Sorting {
    extern std::string slug;
    bool ValidTrade(std::string item, std::vector<std::string> tags);
    bool Frenquency(std::optional<int> rank = std::nullopt);
    bool RankBasedMargin(json orders);
    bool AyatanMargin(json orders);
    bool BasicMargin(json orders);
}
