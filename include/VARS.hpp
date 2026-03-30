#pragma once
#include <optional>
#ifdef _WIN32
#define NOMINMAX
#endif

#include "json.hpp"
#include <any>
#include <atomic>
#include <future>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>

using json = nlohmann::json;
using namespace std;

namespace VARS {
// global generics
inline json items;
inline std::string JWT = "";
// atomics
inline std::atomic<bool> LogLoop{true};
// enums
enum class itemType { basic, mod, Ayatan, platinum };
enum class tradeType { buy, sell };
enum class command { run, set };
// sets
inline std::unordered_set<std::string> RunModes = {"-w", "-d", "-l"};
// maps
inline std::unordered_map<std::string, int> Settings = {
    {"-m", 10}, {"-f", 96}, {"-l", 0}, {"-v", 0}};
// structs
struct rank // stats for a mod at a certain level (only different mod ranks are
            // saved)
{
  int level, sell = std::numeric_limits<int>::max(),
             buy = std::numeric_limits<int>::min();
  bool sell_trade = false, buy_trade = false;
};
struct ayatan_sculpture // stats for an ayatan_sculpture (every iteration is
                        // saved)
{
  int cyanStars, amberStars, buy = std::numeric_limits<int>::min(),
                             sell = std::numeric_limits<int>::max();
  bool sell_trade = false, buy_trade = false;
};
struct basic {
  int sell, buy;
};
struct Trade {
  std::string slug, id;
  int buy, sell;
  itemType Itype;
  tradeType Tstate;
  bool is_user_trade;
  optional<int> level = nullopt, cyanStar = nullopt, amberStar = nullopt;
  int amount = 1;
};

struct Task {
  std::function<json()> work;
  std::promise<json> promise;
};
} // namespace VARS
