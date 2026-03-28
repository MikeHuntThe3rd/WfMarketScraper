#include "OrderHandling.hpp"
#include "CurlReq.hpp"
#include "Trades.hpp"
#include "VARS.hpp"
#include <algorithm>
#include <cctype>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace OrderHandling {
// generic
std::vector<std::string> SeperateBy(std::stringstream &SS, std::string &line,
                                    char separator) {
  std::vector<std::string> result;
  while (std::getline(SS, line, separator)) {
    result.push_back(line);
  }
  return result;
}

std::string Implode(std::vector<std::string> array, char seperator) {
  std::string result = "";
  for (int i = 0; i < array.size(); i++) {
    if (i + 1 == array.size())
      result += array[i];
    else
      result += array[i] + seperator;
  }
  return result;
}

VARS::itemType GetITypeFromSlug(std::string slug) {
  for (const auto &curr : items["data"]) {
    if (curr["slug"] == slug) {
      std::vector<std::string> tags = curr["tags"];

      if (std::find(tags.begin(), tags.end(), "mod") != tags.end() &&
          std::find(tags.begin(), tags.end(), "veiled_riven") == tags.end()) {
        return VARS::itemType::mod;
      } else {
        return VARS::itemType::basic;
      }
    }
  }
  return VARS::itemType::Ayatan;
}

std::string GetIdFromSlug(std::string slug) {
  for (const json &curr : items["data"]) {
    if (curr["slug"] == slug)
      return curr["id"];
  }
  std::cout << "clouldnt find id for: " << "|" << slug << "|" << std::endl;
  return "0";
}

void LowerCase(std::string &word) {
  std::transform(word.begin(), word.end(), word.begin(),
                 [](unsigned char ch) { return std::tolower(ch); });
}

Trade ResultConversion(std::vector<std::string> item, tradeType Ttype) {
  int amount = 1, level = -1;
  std::vector<std::string> data, name;

  // get the name parts of the item
  for (int i = 0; i < item.size(); i++) {
    if (item[i] == "Platinum") {
      amount = stoi(item[i + 2]);
      auto trade =
          Trade{"Platinum", "plat", amount, amount, itemType::platinum, Ttype};
      trade.amount = amount;
      return trade;
    }
    if (item[i] != "x" && item[i].find("(") == std::string::npos) {
      LowerCase(item[i]);
      name.push_back(item[i]);
    } else {
      data.insert(data.end(), item.begin() + i, item.end());
      break;
    }
  }

  for (int i = 0; i < data.size(); i++) {
    if (data[i] == "x") {
      amount = std::stoi(data[i + 1]);
      break;
    }
    if (data[i] == "RANK") {
      level = std::stoi(data[i + 1]);
      break;
    }
  }
  string slug = OrderHandling::Implode(name, '_');
  auto result = Trade{slug, OrderHandling::GetIdFromSlug(slug),    0,
                      0,    OrderHandling::GetITypeFromSlug(slug), Ttype};
  result.amount = amount;
  return result;
}

void EElogChecking() {
  std::ifstream EE("../../out.txt");
  EE.seekg(0, std::ios::beg);
  std::string line;
  std::vector<std::vector<std::string>> soldItems, boughtItems;
  while (VARS::LogLoop) {
    // ifstream
    EE.clear();
    // variables
    std::pair<bool, VARS::tradeType> CheckUnlocked = {false,
                                                      VARS::tradeType::sell};
    // file check by lines
    while (std::getline(EE, line)) {
      // line data gathering
      std::string element = "";
      std::stringstream space{line}, comma{line};
      std::vector<std::string> spaceSeperated, commaSeperated;

      spaceSeperated = OrderHandling::SeperateBy(space, element, ' ');
      commaSeperated = OrderHandling::SeperateBy(comma, element, ',');

      // logic
      if (line.find("and will receive from") != std::string::npos) {
        CheckUnlocked.second = VARS::tradeType::buy;
        goto trade_type_swap;
      }

      if (commaSeperated.size() > 1 && CheckUnlocked.first) {
        CheckUnlocked.first = false;
        std::stringstream ss{*commaSeperated.begin()};
        std::string ln = "";

        boughtItems.push_back(OrderHandling::SeperateBy(ss, ln, ' '));
      }

      if (CheckUnlocked.first && line.length() > 0 &&
          CheckUnlocked.second == VARS::tradeType::sell)
        soldItems.push_back(spaceSeperated);
      else if (CheckUnlocked.first && line.length() > 0 &&
               CheckUnlocked.second == VARS::tradeType::buy)
        boughtItems.push_back(spaceSeperated);

      // state checking
      if (line.find("description=Are you sure you want to accept this trade? "
                    "You are offering:") != std::string::npos) {
        CheckUnlocked = {true, VARS::tradeType::sell};
        soldItems.clear();
        boughtItems.clear();
      }

      // output
      if (line.find("description=The trade was successful!") !=
          std::string::npos) {
        std::vector<Trade> Trades;

        for (auto const &curr : soldItems) {
          Trades.push_back(ResultConversion(curr, tradeType::sell));
        }
        for (auto const &curr : boughtItems) {
          Trades.push_back(ResultConversion(curr, tradeType::buy));
        }

        for (const auto &curr : Trades) {
          std::cout << curr.slug << std::endl;
        }
        // data resetting
        CheckUnlocked = {false, VARS::tradeType::sell};
        soldItems.clear();
        boughtItems.clear();
      }
    trade_type_swap:
    }
    CurlReq::wait();
  }
}

void HandleTrade(Trade trd) {
  json trades_remote =
      CurlReq::q
          .Add([] {
            return CurlReq::__GET("https://api.warframe.market/v2/orders/my",
                                  {"Content-Type: application/json",
                                   "Accept: application/json",
                                   "Authorization: Bearer " + VARS::JWT});
          })
          .get();
  auto map = trades.Read();

  for (json const &order : trades_remote["data"]) {
    if (map.find(order["id"]) != map.end() &&
        map.find(order["id"])->second.slug == trd.slug) {
      cout << "updating: " << trd.slug << endl;

      json del_status = OrderHandling::DeleteOrder(order["id"]);
      if (!del_status.empty() && del_status["error"] == nullptr)
        trades.Remove(del_status["data"]["id"]);
      else {
        cout << "removal failed!" << endl;
        return;
      }

      json post_status = OrderHandling::PostOrder(trd);
      if (!post_status.empty() && post_status["error"] == nullptr)
        trades.Add(post_status["data"]["id"], trd);
      return;
    }
  }

  json post_status = OrderHandling::PostOrder(trd);
  if (!post_status.empty() && post_status["error"] == nullptr)
    trades.Add(post_status["data"]["id"], trd);
}

json DeleteOrder(std::string id) {
  if (id != "") {
    return CurlReq::q
        .Add([id] {
          return CurlReq::__DELETE("https://api.warframe.market/v2/order/" +
                                   id);
        })
        .get();
  } else {
    json orders =
        CurlReq::q
            .Add([] {
              return CurlReq::__GET("https://api.warframe.market/v2/orders/my",
                                    {"Content-Type: application/json",
                                     "Accept: application/json",
                                     "Authorization: Bearer " + VARS::JWT});
            })
            .get();
    for (json order : orders["data"]) {
      CurlReq::q.Add([order] {
        return CurlReq::__DELETE("https://api.warframe.market/v2/order/" +
                                 (std::string)order["id"]);
      });
    }
    return json{};
  }
}

json UpdateOrder(Trade trd) {
  json j;
  j["quantity"] = 1;
  j["visible"] = false;

  switch (trd.Itype) {
  case itemType::basic: {
    if (trd.Tstate == tradeType::buy)
      j["platinum"] = trd.buy;
    else
      j["platinum"] = trd.sell;
    break;
  }
  case itemType::mod: {
    if (trd.Tstate == tradeType::buy)
      j["platinum"] = trd.buy;
    else
      j["platinum"] = trd.sell;
    j["rank"] = trd.level;
    break;
  }
  case itemType::Ayatan: {
    j["perTrade"] = 1;
    break;
  }
  default:
    return json{};
  }
  string id = trd.id;
  return CurlReq::q
      .Add([id, j] {
        return __PATCH("https://api.warframe.market/v2/order/" + id, j.dump());
      })
      .get();
}

json PostOrder(Trade trd) {
  json j;
  j["itemId"] = trd.id;
  j["type"] = (trd.Tstate == tradeType::buy) ? "buy" : "sell";
  j["quantity"] = 1;
  j["visible"] = false;
  // j["perTrade"] = 1;

  switch (trd.Itype) {
  case itemType::basic: {
    if (trd.Tstate == tradeType::buy)
      j["platinum"] = trd.buy;
    else
      j["platinum"] = trd.sell;
    break;
  }
  case itemType::mod: {
    if (trd.Tstate == tradeType::buy)
      j["platinum"] = trd.buy;
    else
      j["platinum"] = trd.sell;
    j["rank"] = trd.level;
    break;
  }
  case itemType::Ayatan: {
    // std::string bdy = R"({
    //     "perTrade": 1
    // })";
    break;
  }
  default:
    return json{};
  }
  return CurlReq::q
      .Add([j] {
        return CurlReq::__POST("https://api.warframe.market/v2/order",
                               j.dump());
      })
      .get();
}
} // namespace OrderHandling
