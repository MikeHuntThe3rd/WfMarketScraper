#pragma once
#ifdef _WIN32
    #define NOMINMAX
#endif
#include <limits>
#include <algorithm>

namespace Types {
    //enums
    enum class itemType {basic, mod, Ayatan};
    enum class tradeType {buy, sell};
    //structs
    struct rank //stats for a mod at a certain level (only different mod ranks are saved)
    {
        bool sell_trade = false, buy_trade = false;
        int sell = std::numeric_limits<int>::max(), buy = std::numeric_limits<int>::min();
    };
    struct ayatan_sculpture //stats for an ayatan_sculpture (every iteration is saved)
    {
        int cyanStars, amberStars,
        buy = std::numeric_limits<int>::min(),
        sell = std::numeric_limits<int>::max();
        bool sell_trade = false, buy_trade = false;
    };
}