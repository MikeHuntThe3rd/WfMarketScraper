#include "Sorting.hpp"
#include "OrderHandling.hpp"
#include "StringOps.hpp"
#include "VARS.hpp"
#include <optional>
namespace Sorting {
std::string slug = "";
std::ofstream logfile;
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
      vol += (int)curr["volume"];
    }
    logfile << "volume min: " << vol << std::endl;
    break;
  case itemType::mod: {
    int rank = std::any_cast<int>(data.value());
    logfile << "----------checking frequency for mod item" << std::endl;
    for (json curr : stats["payload"]["statistics_closed"]["48hours"]) {
      if ((int)curr["mod_rank"] == rank) {
        vol += (int)curr["volume"];
      }
    }
    logfile << "volume min: " << vol << std::endl;
    break;
  }
  case itemType::Ayatan: {
    int cyan = std::any_cast<ayatan_sculpture>(data.value()).cyanStars;
    int amber = std::any_cast<ayatan_sculpture>(data.value()).amberStars;
    logfile << "----------checking frequency for ayatan sculpture" << std::endl;
    for (json curr : stats["payload"]["statistics_closed"]["48hours"]) {
      if ((int)curr["cyan_stars"] == cyan && (int)curr["cyan_stars"] == amber) {
        vol += (int)curr["volume"];
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
std::optional<Trade> RankBasedMargin(json orders) {
  std::vector<VARS::rank> ranks;
  auto find_element = [&ranks](int level) -> VARS::rank & {
    auto iterator =
        std::find_if(ranks.begin(), ranks.end(), [&level](VARS::rank &element) {
          return element.level == level;
        });
    if (iterator != ranks.end()) {
      return *iterator;
    } else {
      ranks.emplace_back(VARS::rank{level});
      return ranks.back();
    }
  };
  for (json curr : orders["data"]) {
    int key;
    try {
      key = curr["rank"];
    } catch (const nlohmann::json::exception &e) {
      cout << "[Attention] rankless mod: " << slug << endl;
      return BasicMargin(orders);
    }

    if (curr["user"]["status"] == "ingame" &&
        curr["platinum"] < find_element(key).sell && curr["type"] == "sell") {
      find_element(key).sell_trade = true;
      find_element(key).sell = curr["platinum"];
    }
    if (curr["user"]["status"] == "ingame" &&
        curr["platinum"] > find_element(key).buy && curr["type"] == "buy") {
      find_element(key).buy_trade = true;
      find_element(key).buy = curr["platinum"];
    }
  }
  int margin = std::numeric_limits<int>::min();
  VARS::rank best;
  bool value_found = false;
  for (const auto &rank : ranks) {
    if ((rank.buy_trade && rank.sell_trade) && rank.sell - rank.buy > margin) {
      margin = rank.sell - rank.buy;
      best = rank;
      value_found = true;
    }
  }
  if (value_found) {
    logfile << "margin: " << margin << std::endl;
    logfile << "rank: " << best.level << std::endl;
  } else
    logfile << "no good mod trade found: " << std::endl;

  if (value_found && margin > VARS::Settings.margin &&
      Frequency(itemType::mod, best.level)) {
    auto trd = Trade{slug,          StringOps::GetIdFromSlug(slug),
                     best.buy,      best.sell,
                     itemType::mod, tradeType::buy,
                     false};
    trd.level = best.level;
    return trd;
  } else {
    return std::nullopt;
  }
}
std::optional<Trade> BasicMargin(json orders) {
  int buy = std::numeric_limits<int>::min();
  int sell = std::numeric_limits<int>::max();
  bool buy_trade = false, sell_trade = false;
  for (json curr : orders["data"]) {
    if (curr["platinum"] < sell && curr["user"]["status"] == "ingame" &&
        curr["type"] == "sell") {
      sell = curr["platinum"];
      sell_trade = true;
    }
    if (curr["platinum"] > buy && curr["user"]["status"] == "ingame" &&
        curr["type"] == "buy") {
      buy = curr["platinum"];
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
std::optional<Trade> AyatanMargin(json orders) {
  std::vector<ayatan_sculpture> sculptures;

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
      continue;
    }

    addIfNew(cyan, amber);

    if (order["user"]["status"] == "ingame" && order["type"] == "sell") {
      priceCompare(cyan, amber, tradeType::sell, order["platinum"]);
    }
    if (order["user"]["status"] == "ingame" && order["type"] == "buy") {
      priceCompare(cyan, amber, tradeType::buy, order["platinum"]);
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
      Frequency(itemType::Ayatan, best)) {
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
} // namespace Sorting
