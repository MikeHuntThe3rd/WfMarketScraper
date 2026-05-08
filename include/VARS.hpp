#pragma once
#include <cstdint>
#include <optional>
#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#endif

#include "json.hpp"
#include <atomic>
#include <future>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>

using json = nlohmann::json;
using namespace std;

namespace VARS {
// atomics
inline std::atomic<bool> thread_run{true};
// enums
enum class itemType { basic, mod, Ayatan };
enum class tradeType { buy, sell };
enum class command { run, set };
// structs
struct costum_exit {
  int code;
  string msg;

  costum_exit(int code, string msg) : code(code), msg(msg) {}
};
struct Settings_template {
  int margin;
  int freq;
  int plat;
  int offset;
  bool log;
  bool vt;
  bool dos;
  string jwt;
  string ee_path;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings_template, margin, freq, plat,
                                   offset, log, vt, dos, jwt, ee_path)
struct rank // stats for a mod at a certain level (only different mod ranks are
            // saved)
{
  int sell = std::numeric_limits<int>::max(),
      buy = std::numeric_limits<int>::min();
  bool sell_trade = false, buy_trade = false;
};
struct ayatan_sculpture // stats for an ayatan_sculpture (every iteration is
                        // saved)
{
  int cyanStars, amberStars, buy = std::numeric_limits<int>::min(),
                             sell = std::numeric_limits<int>::max();
  bool sell_trade = false, buy_trade = false;

  ayatan_sculpture(int cyan, int amber) : cyanStars(cyan), amberStars(amber) {};
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
  std::optional<int> level = std::nullopt, cyanStar = std::nullopt,
                     amberStar = std::nullopt;
  int amount = 1;

  Trade(std::string slug, std::string id, int buy, int sell, itemType itype,
        tradeType ttype, bool is_user_trade)
      : slug(slug), id(id), buy(buy), sell(sell), Itype(itype), Tstate(ttype),
        is_user_trade(is_user_trade) {}
};
struct Task {
  std::function<json()> work;
  std::promise<json> promise;
};
// global generics
inline json items;
inline std::string JWT = "";
inline const json DefSettings{{"margin", 10}, {"freq", 96},   {"plat", 0},
                              {"offset", 1},  {"log", false}, {"vt", false},
                              {"dos", false}, {"jwt", ""}, {"ee_path", ""}};
inline Settings_template Settings;
} // namespace VARS
