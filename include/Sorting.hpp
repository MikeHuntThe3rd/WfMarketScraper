#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <limits>
#include <optional>
#include "json.hpp"
#include "CurlReq.hpp"

using json = nlohmann::json;
extern std::string slug;
bool CheckMargin(std::string item);
bool TradeFrenquency(std::optional<int> rank = std::nullopt);
bool RankBasedMargin(json orders);