#include "Sorting.hpp"
#include "OrderHandling.hpp"
#include "StringOps.hpp"
#include "VARS.hpp"
#include <optional>
#include <unordered_map>
namespace Sorting {
std::string slug = "";
std::ofstream logfile;

//trade validation
bool Frequency(itemType type, std::optional<std::any> data) {
  json stats =
      CurlReq::q
          .Add([] {
            return CurlReq::__GET("https://api.warframe.market/v1/items/" +
                                  (std::string)slug + "/statistics");
          })
          .get();
  int vol = 0;
  switch (type) {
  case itemType::basic:
    logfile << "----------checking frequency for basic item" << std::endl;
    for (json curr : stats["payload"]["statistics_closed"]["48hours"]) {
      vol += curr["volume"].get<int>();
    }
    logfile << "volume min: " << vol << std::endl;
    break;
  case itemType::mod: {
    int rank = std::any_cast<int>(data.value());
    logfile << "----------checking frequency for mod item" << std::endl;
    for (json curr : stats["payload"]["statistics_closed"]["48hours"]) {
      if ((int)curr["mod_rank"] == rank) {
        vol += curr["volume"].get<int>();
      }
    }
    logfile << "volume min: " << vol << std::endl;
    break;
  }
  case itemType::Ayatan: {
    auto sculpture = any_cast<ayatan_sculpture>(data.value());
    logfile << "----------checking frequency for ayatan sculpture" << std::endl;
    for (json curr : stats["payload"]["statistics_closed"]["48hours"]) {
      if (curr["cyan_stars"].get<int>() == sculpture.cyanStars &&
          curr["cyan_stars"].get<int>() == sculpture.amberStars) {
        vol += curr["volume"].get<int>();
      }
    }
    logfile << "volume min: " << vol << std::endl;
    break;
  }
  default:
    break;
  }

  if (vol > VARS::Settings.freq) {
    return true;
  } else {
    return false;
  }
}

optional<Trade> RankBasedMargin(json orders) {
  unordered_map<int, VARS::rank> ranks;
  auto addIfNew = [&ranks](int level) -> void {
    auto iter = ranks.find(level);
    if (iter == ranks.end())
      ranks.insert({level, VARS::rank{}});
  };
  for (json curr : orders["data"]) {
    int key;
    try {
      key = curr["rank"];
    } catch (const nlohmann::json::exception &e) {
      return BasicMargin(orders);
    }
    addIfNew(key);
    auto iter = ranks.find(key);
    auto value = &iter->second;

    if (curr["user"]["status"] == "ingame" &&
        curr["platinum"].get<int>() - VARS::Settings.offset < value->sell &&
        curr["type"] == "sell" && curr["user"]["id"].get<string>() != VARS::user_id) {
      value->sell_trade = true;
      value->sell = curr["platinum"].get<int>() - VARS::Settings.offset;
    }
    if (curr["user"]["status"] == "ingame" &&
        curr["platinum"].get<int>() + VARS::Settings.offset > value->buy &&
        curr["type"] == "buy" && curr["user"]["id"].get<string>() != VARS::user_id) {
      value->buy_trade = true;
      value->buy = curr["platinum"].get<int>() + VARS::Settings.offset;
    }
  }
  int margin = std::numeric_limits<int>::min(), level;
  VARS::rank best{};
  bool value_found = false;
  for (const auto &rank : ranks) {
    auto key = rank.first;
    auto value = rank.second;
    bool good = (value.buy_trade && value.sell_trade);
    bool better = value.sell - value.buy > margin;
    if (good && better) {
      margin = value.sell - value.buy;
      best = value;
      level = key;
      value_found = true;
    }
  }
  if (value_found) {
    logfile << "margin: " << margin << std::endl;
    logfile << "rank: " << level << std::endl;
  } else
    logfile << "no good mod trade found: " << std::endl;

  if (value_found && margin > VARS::Settings.margin &&
      Frequency(itemType::mod, level)) {
    auto trd = Trade{slug,          StringOps::GetIdFromSlug(slug),
                     best.buy,      best.sell,
                     itemType::mod, tradeType::buy,
                     false};
    trd.level = level;
    return trd;
  } else {
    return std::nullopt;
  }
}

optional<Trade> BasicMargin(json orders) {
  int buy = std::numeric_limits<int>::min();
  int sell = std::numeric_limits<int>::max();
  bool buy_trade = false, sell_trade = false;
  
  for (json curr : orders["data"]) {
    if (curr["user"]["status"] == "ingame" &&
        curr["platinum"].get<int>() - VARS::Settings.offset < sell &&
        curr["type"] == "sell" && curr["user"]["id"].get<string>() != VARS::user_id) {
      sell = curr["platinum"].get<int>() - VARS::Settings.offset;
      sell_trade = true;
    }
    if (curr["user"]["status"] == "ingame" &&
        curr["platinum"].get<int>() + VARS::Settings.offset > buy &&
        curr["type"] == "buy" && curr["user"]["id"].get<string>() != VARS::user_id) {
      buy = curr["platinum"].get<int>() + VARS::Settings.offset;
      buy_trade = true;
    }
  }
  logfile << "margin: " << sell - buy << std::endl;
  if ((sell_trade && buy_trade) && sell - buy > VARS::Settings.margin &&
      Frequency(itemType::basic)) {
    return Trade{slug,
                 StringOps::GetIdFromSlug(slug),
                 buy,
                 sell,
                 itemType::basic,
                 tradeType::buy,
                 false};
  } else {
    return std::nullopt;
  }
}

optional<Trade> AyatanMargin(json orders) {
  vector<ayatan_sculpture> sculptures;

  auto findElement = [&sculptures](int cyanStars, int amberStars) {
    auto match =
        find_if(sculptures.begin(), sculptures.end(),
                [&cyanStars, &amberStars](const ayatan_sculpture &sculpture) {
                  return sculpture.cyanStars == cyanStars &&
                         sculpture.amberStars == amberStars;
                });
    return match;
  };
  auto addIfNew = [&sculptures, findElement](int cyanStars, int amberStars) {
    if (findElement(cyanStars, amberStars) == sculptures.end()) {
      sculptures.push_back({cyanStars, amberStars});
    }
  };
  auto priceCompare = [&sculptures, findElement](int cyanStars, int amberStars,
                                                 tradeType type, int platinum) {
    auto element = findElement(cyanStars, amberStars);
    if (element != sculptures.end()) {
      switch (type) {
      case tradeType::sell:
        if (element->sell > platinum) {
          element->sell = platinum;
          element->sell_trade = true;
        }
        break;
      case tradeType::buy:
        if (element->buy < platinum) {
          element->buy = platinum;
          element->buy_trade = true;
        }
        break;
      }
    }
  };

  for (json order : orders["data"]) {
    int cyan = 0;
    int amber = 0;
    try {
      amber = order["amberStars"];
      cyan = order["cyanStars"];
    } catch (...) {
      cout << "[Error] 'cyanStars' or 'amberStars' variable(s) not found"
           << endl;
      return nullopt;
    }

    addIfNew(cyan, amber);

    if (order["user"]["status"] == "ingame" && order["type"] == "sell" && order["user"]["id"].get<string>() != VARS::user_id) {
      priceCompare(cyan, amber, tradeType::sell,
                   order["platinum"].get<int>() - VARS::Settings.offset);
    }
    if (order["user"]["status"] == "ingame" && order["type"] == "buy" && order["user"]["id"].get<string>() != VARS::user_id) {
      priceCompare(cyan, amber, tradeType::buy,
                   order["platinum"].get<int>() + VARS::Settings.offset);
    }
  }

  int margin = numeric_limits<int>::min();
  optional<ayatan_sculpture> best = nullopt;
  for (ayatan_sculpture SC : sculptures) {
    if ((SC.sell_trade && SC.buy_trade) && SC.sell - SC.buy > margin) {
      margin = SC.sell - SC.buy;
      best = SC;
    }
  }
  logfile << "resulting margin: " << margin << endl;
  logfile << "seperate sculptures: " << sculptures.size() << endl;
  if (best.has_value())
    logfile << "struct of the best sculpture (stars: c, a):" << best->cyanStars
            << ", " << best->amberStars << endl;
  else
    logfile << "best is empty" << endl;

  if (best.has_value() && margin > VARS::Settings.margin &&
      Frequency(itemType::Ayatan, best.value())) {
    return Trade{slug,
                 StringOps::GetIdFromSlug(slug),
                 best.value().buy,
                 best.value().sell,
                 itemType::Ayatan,
                 tradeType::buy,
                 false};
  } else {
    return nullopt;
  }
}

optional<Trade> ValidTrade(string item, vector<string> tags, bool log) {
  slug = item;
  optional<Trade> __return;
  if (log) {
    logfile.open("out.log", std::ios::app);
  }
  json orders = CurlReq::q
                    .Add([] {
                      return CurlReq::__GET(
                          "https://api.warframe.market/v2/orders/item/" +
                          (std::string)slug);
                    })
                    .get();
  if (std::find(tags.begin(), tags.end(), "ayatan_sculpture") != tags.end()) {
    logfile << "==========AYATAN CHECK==========" << std::endl;
    logfile << "slug: " << slug << std::endl;

    auto result = AyatanMargin(orders);
    if (result.has_value()) {
      logfile << "[Success] found good trade for: " << slug << std::endl;
      cout << "[Success] found good trade for: " << slug << std::endl;
    }
    __return = result;

  } else if (std::find(tags.begin(), tags.end(), "mod") != tags.end() &&
             std::find(tags.begin(), tags.end(), "veiled_riven") ==
                 tags.end()) {
    logfile << "==========MOD CHECK==========" << std::endl;
    logfile << "slug: " << slug << std::endl;

    auto result = RankBasedMargin(orders);
    if (result.has_value()) {
      logfile << "[Success] found good trade for: " << slug << std::endl;
      cout << "[Success] found good trade for: " << slug << std::endl;
    }

    __return = result;
  } else {
    logfile << "==========BASIC CHECK==========" << std::endl;
    logfile << "slug: " << slug << std::endl;

    auto result = BasicMargin(orders);
    if (result.has_value()) {
      logfile << "[Success] found good trade for: " << slug << std::endl;
      cout << "[Success] found good trade for: " << slug << std::endl;
    }
    __return = result;
  }
  logfile.close();
  return __return;
}

//trade tracking
void SetBestTradePrice(VARS::Trade &trd){
  json orders = CurlReq::q
                    .Add([trd] {
                      return CurlReq::__GET(
                          "https://api.warframe.market/v2/orders/item/" +
                          (string)trd.slug);
                    })
                    .get();
  
  int sell = numeric_limits<int>::max(), buy = numeric_limits<int>::min();;
  for(json const &order : orders["data"]){
    if(order["user"]["status"] != "ingame" || order["user"]["id"] == VARS::user_id) continue;

    switch (trd.Itype)
    {
      case VARS::itemType::mod:
      if(trd.level != order["rank"]) continue;
      break;

      case VARS::itemType::Ayatan:
      if(trd.amberStar != order["amberStars"] || trd.cyanStar != order["cyanStars"]) continue;
      break;

      default:
      break;
    }

    if(order["type"] == "sell" && order["platinum"] <= sell){
      sell = order["platinum"].get<int>() - VARS::Settings.offset;
    }
    if(order["type"] == "buy" && order["platinum"] >= buy){
      buy = order["platinum"].get<int>() + VARS::Settings.offset;
    }
  }
  if(sell != numeric_limits<int>::max()) trd.sell = sell;
  if(buy != numeric_limits<int>::min()) trd.buy = buy;
}

} // namespace Sorting
