#include "OrderHandling.hpp"
#include "CurlReq.hpp"
#include "Trades.hpp"
#include "VARS.hpp"
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace OrderHandling {
void HandlelocalTrade(vector<string> item, tradeType trade_state) {
  std::vector<std::string> name, data;

  // get the name parts of the item
  for (int i = 0; i < item.size(); i++) {
    if (item[i] == "Platinum") {

      uint32_t pl_amount = static_cast<uint32_t>(stoi(item[i + 2]));
      switch (trade_state) {
      case VARS::tradeType::sell:
        VARS::platinum -= pl_amount;
      case VARS::tradeType::buy:
        VARS::platinum += pl_amount;
      }
      return;
    }
    if (item[i] != "x" && item[i].find("(") == std::string::npos) {
      LowerCase(item[i]);
      name.push_back(item[i]);
    } else {
      data.insert(data.end(), item.begin() + i, item.end());
      break;
    }
  }

  int amount, level;
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

  string slug = Implode(name, '_');
  for (const auto &order : Trds::trades.Read()) {
    if (order.second.slug == slug && order.second.Tstate == trade_state) {
      switch (trade_state) {
      case VARS::tradeType::sell: {
        cout << "[Deleting] " << slug << endl;

        DeleteOrder(order.first);
        Trds::trades.Remove(order.first);
        return;
      }
      case VARS::tradeType::buy: {
        cout << "[Updating] " << slug << endl;

        auto updated_trd = order.second;
        updated_trd.Tstate = VARS::tradeType::sell;
        json post_status = OrderHandling::PostOrder(updated_trd);
        if (!post_status.empty() && post_status["error"] == nullptr) {
          OrderHandling::DeleteOrder(order.first);
          Trds::trades.Set(order.first, updated_trd);
        } else {
          cout << "[Error] update failed" << endl;
        }
        return;
      }
      }
    } else {
    }
  }
}

void EElogChecking() {
  std::ifstream EE("../../out.txt");
  EE.seekg(0, std::ios::end);
  std::string line;
  std::vector<std::vector<std::string>> soldItems, boughtItems;

  // test append
  // ofstream fil("../../out.txt", ios::app);
  // fil << ifstream("../../inp.txt").rdbuf();
  // fil.close();

  while (VARS::thread_run) {
    // ifstream
    EE.clear();
    // variables
    std::pair<bool, VARS::tradeType> CheckUnlocked = {false,
                                                      VARS::tradeType::sell};

    // file check by lines
    while (getline(EE, line)) {
      // line data gathering
      std::string element = "";
      std::stringstream space{line}, comma{line};
      std::vector<std::string> spaceSeperated, commaSeperated;

      spaceSeperated = StringOps::SeperateBy(space, element, ' ');
      commaSeperated = StringOps::SeperateBy(comma, element, ',');

      // logic
      if (line.find("and will receive from") != std::string::npos) {
        CheckUnlocked.second = VARS::tradeType::buy;
        goto trade_type_swap;
      }

      if (commaSeperated.size() > 1 && CheckUnlocked.first) {
        CheckUnlocked.first = false;
        std::stringstream ss{*commaSeperated.begin()};
        std::string ln = "";

        boughtItems.push_back(StringOps::SeperateBy(ss, ln, ' '));
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
        Trds::trades.Sync(CurlReq::q
                              .Add([] {
                                return CurlReq::__GET(
                                    "https://api.warframe.market/v2/orders/my",
                                    {"Content-Type: application/json",
                                     "Accept: application/json",
                                     "Authorization: Bearer " + VARS::JWT});
                              })
                              .get());

        for (auto const &curr : soldItems) {
          HandlelocalTrade(curr, tradeType::sell);
        }
        for (auto const &curr : boughtItems) {
          HandlelocalTrade(curr, tradeType::buy);
        }

        // data resetting
        CheckUnlocked = {false, VARS::tradeType::sell};
        soldItems.clear();
        boughtItems.clear();
      }
    trade_type_swap:;
    }
    CurlReq::wait();
  }
}

void HandleNewTrade(Trade trd) {
  json trades_remote =
      CurlReq::q
          .Add([] {
            return CurlReq::__GET("https://api.warframe.market/v2/orders/my",
                                  {"Content-Type: application/json",
                                   "Accept: application/json",
                                   "Authorization: Bearer " + VARS::JWT});
          })
          .get();
  Trds::trades.Sync(trades_remote);
  auto map = Trds::trades.Read();

  for (json const &order : trades_remote["data"]) {
    if (map.find(order["id"]) != map.end() &&
        map.find(order["id"])->second.slug == trd.slug) {
      cout << "[Updating] " << trd.slug << endl;

      json update_status = OrderHandling::UpdateOrder(order["id"], trd);
      if (!update_status.empty() && update_status["error"] == nullptr)
        Trds::trades.Set(order["id"], trd);
      else {
        cout << "[Error] update failed" << endl;
        return;
      }
    }
  }

  json post_status = OrderHandling::PostOrder(trd);
  if (!post_status.empty() && post_status["error"] == nullptr)
    Trds::trades.Add(post_status["data"]["id"], trd);
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

json UpdateOrder(string trade_id, Trade trd) {
  json j;
  j["quantity"] = 1;
  j["visible"] = VARS::Settings.vt;

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
  return CurlReq::q
      .Add([trade_id, j] {
        return __PATCH("https://api.warframe.market/v2/order/" + trade_id,
                       j.dump());
      })
      .get();
}

json PostOrder(Trade trd) {
  json j;
  j["itemId"] = trd.id;
  j["type"] = (trd.Tstate == tradeType::buy) ? "buy" : "sell";
  j["quantity"] = 1;
  j["visible"] = VARS::Settings.vt;
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