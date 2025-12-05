#pragma once
#ifdef _WIN32
    #define NOMINMAX
#endif
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
    //variables
    extern std::string slug;
    enum class itemType {basic, mod, Ayatan};
    enum class tradeType {buy, sell};
    struct rank //stats for a mod at a certain level (only different mod ranks are saved)
    {
        bool sell_trade = false, buy_trade = false;
        int sell = std::numeric_limits<int>::max(), buy = std::numeric_limits<int>::min();
    };
    struct ayatan_sculpture //stats for an ayatan_sculpture (every iteration is saved)
    {
        int cyanStars, amberStars,
        buy = std::numeric_limits<int>::min(),
        sell = std::numeric_limits<int>::max();;
    };
    //functions
    bool ValidTrade(std::string item, std::vector<std::string> tags);
    bool Frequency(itemType type, std::optional<std::any> data = std::nullopt);
    bool RankBasedMargin(json orders);
    bool AyatanMargin(json orders);
    bool BasicMargin(json orders);
}
