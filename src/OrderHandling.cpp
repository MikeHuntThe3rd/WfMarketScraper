#include "OrderHandling.hpp"
#include "CurlReq.hpp"
#include "Sorting.hpp"
#include "StringOps.hpp"
#include "Trades.hpp"
#include "VARS.hpp"
#include <fstream>
#include <ios>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace OrderHandling {

local_trade ParseSingleTrade(vector<string> data, VARS::tradeType type) {
  vector<string> slug;
  int amount = 1;
  for (int i = 0; i < data.size(); i++) {
    if (data[i] == "Platinum") {
      int pl_amount = (int)(stoi(data[i + 2]));
      return local_trade{"Platinum", type, pl_amount};
    }
    if (data[i] == "x") {
      amount = (int)(stoi(data[i + 2]));
      break;
    } else if (data[i].find("(") != string::npos) {
      break;
    } else {
      StringOps::LowerCase(data[i]);
      slug.push_back(data[i]);
    }
  }
  string slg = StringOps::Implode(slug, '_');
  return local_trade{StringOps::GetIdFromSlug(slg), type, amount};
}

optional<local_trade> HandleSet(vector<local_trade> &trades,
                                local_trade current, json orders) {
  string root;
  vector<string> parts;
  json set =
      CurlReq::q
          .Add([current]() {
            return CurlReq::__GET("https://api.warframe.market/v2/item/" +
                                  current.id + "/set");
          })
          .get();
  if (set["data"]["items"].size() == 1)
    return nullopt;

  for (json const &item : set["data"]["items"]) {
    if (item["setRoot"].get<bool>() == true)
      root = item["id"].get<string>();
    else
      parts.push_back(item["id"].get<string>());
  }
  for (json const &order : orders["data"]) {
    if (order["itemId"].get<string>() == root)
      goto set_found;
  }
  return nullopt;
set_found:;

  for (string const &part : parts) {
    for (auto iter = trades.begin(); iter != trades.end(); iter++) {
      if (iter->id == part && iter->type == current.type) {
        trades.erase(iter);
        break;
      }
    }
  }
  return local_trade{root, current.type, 1};
}

vector<local_trade> ParseLocalTrades(vector<vector<string>> soldItems,
                                     vector<vector<string>> boughtItems,
                                     json orders) {
  vector<local_trade> trades;
  if (orders["data"].size() == 0)
    return trades;
  for (auto const &sold : soldItems) {
    trades.push_back(ParseSingleTrade(sold, VARS::tradeType::sell));
  }

  for (auto const &bought : boughtItems) {
    trades.push_back(ParseSingleTrade(bought, VARS::tradeType::buy));
  }

  for (auto l_trade = trades.begin(); l_trade != trades.end();) {
    bool trade_found = false;
    for (json const &order : orders["data"]) {
      VARS::tradeType t_type = (order["type"].get<string>() == "buy")
                                   ? VARS::tradeType::buy
                                   : VARS::tradeType::sell;
      if ((order["itemId"].get<string>() == l_trade->id &&
           t_type == l_trade->type) ||
          l_trade->id == "Platinum") {
        trade_found = true;
        break;
      }
    }
    if (!trade_found) {
      auto is_set = HandleSet(trades, *l_trade, orders);
      if (is_set.has_value()) {
        trades.push_back(is_set.value());
        l_trade++;
      } else
        l_trade = trades.erase(l_trade);
    } else
      l_trade++;
  }
  return trades;
}

void HandlelocalTrade(local_trade trd) {
  if (trd.id == "Platinum") {
    json sett = json::parse(ifstream("settings.json"));
    switch (trd.type) {
    case VARS::tradeType::sell: {
      sett["plat"] = sett["plat"].get<int>() - trd.amount;
      break;
    }
    case VARS::tradeType::buy: {
      sett["plat"] = sett["plat"].get<int>() + trd.amount;
      break;
    }
    }
    ofstream("settings.json", ios::trunc) << sett.dump(4);
    return;
  }

  for (const auto &order : Trds::trades.Read()) {
    if (order.second.id == trd.id && order.second.Tstate == trd.type) {
      string slug = StringOps::GetSlugFromId(trd.id);
      switch (trd.type) {
      case VARS::tradeType::sell: {
        cout << "[Deleting] " << slug << endl;

        DeleteOrder(order.first);
        Trds::trades.Remove(order.first);
        return;
      }
      case VARS::tradeType::buy: {
        cout << "[Moving] " << slug << endl;

        auto updated_trd = order.second;
        updated_trd.Tstate = VARS::tradeType::sell;
        json post_status = OrderHandling::PostOrder(updated_trd);
        if (!post_status.empty() && post_status["error"] == nullptr) {
          OrderHandling::DeleteOrder(order.first);
          Trds::trades.Set(order.first, updated_trd);
        } else {
          cout << "[Error] move failed" << endl;
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
  string line;
  vector<vector<string>> soldItems, boughtItems;

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
      string element = "";
      stringstream space{line}, comma{line};
      vector<string> spaceSeperated, commaSeperated;

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

      if (line.find("SendResult_MENU_CANCEL()") != string::npos) {
        CheckUnlocked = {false, VARS::tradeType::sell};
        soldItems.clear();
        boughtItems.clear();
      }

      // closing the check on trade success
      if (line.find("description=The trade was successful!") !=
          std::string::npos) {
        json orders = CurlReq::q
                          .Add([] {
                            return CurlReq::__GET(
                                "https://api.warframe.market/v2/orders/my",
                                {"Content-Type: application/json",
                                 "Accept: application/json",
                                 "Authorization: Bearer " + VARS::JWT});
                          })
                          .get();
        Trds::trades.Sync(orders);

        auto trds = ParseLocalTrades(soldItems, boughtItems, orders);
        for (auto const &trd : trds) {
          HandlelocalTrade(trd);
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

json FindItem(string id) {
  for (const json &curr : VARS::items["data"]) {
    if (curr["id"] == id)
      return curr;
  }
  return json{};
}

void ManageOneTrade(json order) {
  auto map = Trds::trades.Read();
  auto iter = map.find(order["id"]);
  if (iter == map.end()) {
    cout << "[Error] trades doesn't contain the provided id: " << order["id"]
         << endl;
    return;
  }
  Trade remote_trade = iter->second;
  json item = FindItem(remote_trade.id);

  auto validated_trade =
      Sorting::ValidTrade(item["slug"], item["tags"], VARS::Settings.log);
  if (!validated_trade.has_value() &&
      remote_trade.Tstate == VARS::tradeType::buy) {
    cout << "[Deleting] no good trade found for: " << remote_trade.slug << endl;
    json delete_status = OrderHandling::DeleteOrder(order["id"]);

    if (!delete_status.empty() && delete_status["error"] == nullptr) {
      Trds::trades.Remove(order["id"]);
      return;
    } else {
      cout << "[Error] delete failed" << endl;
      return;
    }
  }
  if (validated_trade.has_value() &&
      VARS::Settings.plat - validated_trade->buy >= 0) {
    remote_trade.buy = validated_trade->buy;
    remote_trade.sell = validated_trade->sell;
    cout << "[Updating] " << remote_trade.slug << endl;

    json update_status = OrderHandling::UpdateOrder(order["id"], remote_trade);

    if (!update_status.empty() && update_status["error"] == nullptr) {
      Trds::trades.Set(order["id"], remote_trade);
      return;
    } else {
      cout << "[Error] update failed" << endl;
      return;
    }
  }
}

void HandleAllTrades() {
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

  for (const json &order : trades_remote["data"]) {
    ManageOneTrade(order);
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
  auto iter = map.end();

  for (auto it = map.begin(); it != map.end(); it++) {
    if (it->second.slug == trd.slug &&
        it->second.Tstate == VARS::tradeType::buy) {
      iter = it;
      break;
    }
  }

  if (iter != map.end()) {
    cout << "[Updating] " << trd.slug << endl;
    UpdateOrder(iter->first, trd);
  } else {
    cout << "[Posting] " << trd.slug << endl;
    PostOrder(trd);
  }
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
    j["amberStars"] = trd.amberStar;
    j["cyanStars"] = trd.cyanStar;
    break;
  }
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
    if (trd.Tstate == tradeType::buy)
      j["platinum"] = trd.buy;
    else
      j["platinum"] = trd.sell;

    j["perTrade"] = 1;
    j["amberStars"] = trd.amberStar;
    j["cyanStars"] = trd.cyanStar;
    break;
  }
  }
  return CurlReq::q
      .Add([j] {
        return CurlReq::__POST("https://api.warframe.market/v2/order",
                               j.dump());
      })
      .get();
}
} // namespace OrderHandling
