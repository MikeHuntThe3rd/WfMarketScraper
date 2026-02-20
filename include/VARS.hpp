#pragma once
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
    {"-m", 10}, {"-f", 96}, {"-l", 0}};
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
  bool good_trade;
  std::string slug, id;
  itemType Itype;
  tradeType Ttype;
  std::any data;
};
struct local_trade {
  tradeType Ttype;
  itemType Itype;
  std::string slug = "empty slug", id = "empty id";
  int amount = 1, level = 0;
};
struct Task {
  std::function<json()> work;
  std::promise<json> promise;
};
} // namespace VARS
