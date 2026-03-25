#include "OrderHandling.hpp"
#include "CurlReq.hpp"
#include "VARS.hpp"
#include <algorithm>
#include <cctype>
#include <ios>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
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

local_trade ResultConversion(std::vector<std::string> item, tradeType Ttype) {
  VARS::local_trade result = VARS::local_trade{Ttype, VARS::itemType::basic};
  std::vector<std::string> name;
  std::vector<std::string> data;
  for (int i = 0; i < item.size(); i++) {
    if (item[i] == "Platinum") {
      result = VARS::local_trade{Ttype, itemType::platinum, "plat"};
      data.insert(data.end(), item.begin() + i, item.end());
      break;
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
      result.amount = std::stoi(data[i + 1]);
      break;
    }
    if (data[i] == "RANK") {
      result.level = std::stoi(data[i + 1]);
      break;
    }
  }
  if (result.Itype != itemType::platinum) {
    result.slug = Implode(name, '_');
    result.id = GetIdFromSlug(result.slug);
    result.Itype = GetITypeFromSlug(result.slug);
  }
  return result;
}

void HandleTrades(std::vector<local_trade> Trades) {
  json orders =
      CurlReq::q
          .Add([] {
            return CurlReq::__GET("https://api.warframe.market/v2/orders/my",
                                  {"Content-Type: application/json",
                                   "Accept: application/json",
                                   "Authorization: Bearer " + VARS::JWT});
          })
          .get();
  for (local_trade &trade : Trades) {
    switch (trade.Ttype) {
    case VARS::tradeType::sell:
      HandleSell(trade, orders);
      break;
    case VARS::tradeType::buy:
      HandleBuy(trade, orders);
      break;
    }
  }
}

void HandleSell(local_trade &trade, json &orders) {
  for (const auto &order : orders) {
    if (order["id"] == trade.id) {
    }
  }
}

void HandleBuy(local_trade &trade, json &orders) {
  for (const auto &order : orders) {
    if (order["id"] == trade.id) {
      // exec
    }
  }
}

void EElogChecking() {
  std::ifstream EE("../../out.txt");
  EE.seekg(0, std::ios::end);
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
        std::vector<local_trade> Trades;

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

void DeleteOrder(std::string id) {
  if (id != "") {
    CurlReq::q.Add([id] {
      return CurlReq::__DELETE("https://api.warframe.market/v2/order/" + id);
    });
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
  }
}

void UpdateOrder(std::string id, itemType type, tradeType trade_type,
                 std::any data) {
  json j;
  j["quantity"] = 1;
  j["visible"] = false;

  switch (type) {
  case itemType::basic: {
    if (trade_type == tradeType::buy)
      j["platinum"] = std::any_cast<basic>(data).buy;
    else
      j["platinum"] = std::any_cast<basic>(data).sell;
    break;
  }
  case itemType::mod: {
    if (trade_type == tradeType::buy)
      j["platinum"] = std::any_cast<rank>(data).buy;
    else
      j["platinum"] = std::any_cast<rank>(data).sell;
    j["rank"] = std::any_cast<rank>(data).level;
    break;
  }
  case itemType::Ayatan: {
    j["perTrade"] = 1;
    break;
  }
  }

  CurlReq::q.Add([id, j] {
    return __PATCH("https://api.warframe.market/v2/order/" + id, j.dump());
  });
}

void PostOrder(std::string id, tradeType type, Trade trade) {
  json j;
  j["itemId"] = id;
  j["type"] = (type == tradeType::buy) ? "buy" : "sell";
  j["quantity"] = 1;
  j["visible"] = false;
  // j["perTrade"] = 1;

  switch (trade.Itype) {
  case itemType::basic: {
    if (type == tradeType::buy)
      j["platinum"] = std::any_cast<basic>(trade.data).buy;
    else
      j["platinum"] = std::any_cast<basic>(trade.data).sell;
    break;
  }
  case itemType::mod: {
    if (type == tradeType::buy)
      j["platinum"] = std::any_cast<rank>(trade.data).buy;
    else
      j["platinum"] = std::any_cast<rank>(trade.data).sell;
    j["rank"] = std::any_cast<rank>(trade.data).level;
    break;
  }
  case itemType::Ayatan: {
    // std::string bdy = R"({
    //     "perTrade": 1
    // })";
    break;
  }
  }
  CurlReq::q.Add([j] {
    return CurlReq::__POST("https://api.warframe.market/v2/order", j.dump());
  });
}
} // namespace OrderHandling
