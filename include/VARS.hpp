#pragma once
#include "json.hpp"
#include <atomic>
#include <future>
#include <limits>
#include <string>
#include <cstdint>
#include <fstream>
#include <mutex>
#include <optional>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#endif

using json = nlohmann::json;
using namespace std;

namespace VARS {
// atomics
inline std::atomic<bool> thread_run{true};
inline std::mutex log_lock, settings_lock;

// enums
enum class itemType { basic, mod, Ayatan };
enum class tradeType { buy, sell };
enum class command { run, set };

// structs
struct costum_exit {
  int code;
  std::string msg;

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
extern const std::filesystem::path settings_path, log_path;
inline json items;
inline std::string JWT = "";
inline std::string user_id = "";
inline const json DefSettings{{"margin", 10}, {"freq", 96},   {"plat", 0},
                              {"offset", 1},  {"log", false}, {"vt", false},
                              {"dos", false}, {"jwt", ""},    {"ee_path", ""}};
inline Settings_template Settings;

// functions
inline void WriteToLog(string line) {
  if (!Settings.log)
    return;

  if(!std::filesystem::exists(log_path)) throw VARS::costum_exit{1, "[error] couldnt find path for logs"};
  
  std::lock_guard<mutex> lock(log_lock);
  std::ofstream log("out.log", std::ios::app);
  log << line << std::endl;
}

inline json GetSettings(){
  if(!std::filesystem::exists(settings_path)) throw VARS::costum_exit{1, "[error] couldnt find path for settings"};

  std::lock_guard<mutex> lock(settings_lock);
  std::ifstream file(settings_path);

  return json::parse(file);
}

inline void SetSettings(const json& new_settings){
  if(!std::filesystem::exists(settings_path)) throw VARS::costum_exit{1, "[error] couldnt find path for settings"};

  std::lock_guard<mutex> lock(settings_lock);
  std::ofstream file(settings_path, std::ios::trunc);

  file << new_settings.dump(4);
}

} // namespace VARS
