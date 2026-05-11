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

#ifdef _WIN32
#pragma once
#include "VDFparser.hpp"
#include <windows.h>

optional<string> FindEElogPath() {
  filesystem::path full_path;
  HKEY key;
  char steam_path[MAX_PATH], appdata_path[MAX_PATH];
  DWORD steam_size = sizeof(steam_path), appdata_size = sizeof(appdata_path);
  // try most common location
  RegGetValueA(HKEY_CURRENT_USER, "Volatile Environment", "LOCALAPPDATA",
               RRF_RT_REG_SZ, nullptr, appdata_path, &appdata_size);
  full_path = filesystem::path(appdata_path) / "Warframe" / "EE.log";
  if (filesystem::exists(full_path))
    return full_path.string();

  // find the steam managed warframe folder in case EE.log wasnt in appdata
  RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Valve\\Steam",
               "InstallPath", RRF_RT_REG_SZ, nullptr, steam_path, &steam_size);
  filesystem::path vdf_path =
      filesystem::path(steam_path) / "config" / "libraryfolders.vdf";
  if (!filesystem::exists(vdf_path))
    return nullopt;

  auto root = Parser::Parse<unordered_map>(ifstream(vdf_path));
  if (!root.has_value() || root.value().childern.size() == 0)
    return nullopt;
  auto lib_iter = root.value().childern.find("libraryfolders");
  if (lib_iter == root.value().childern.end())
    return nullopt;
  auto vdf_obj = root.value().childern["libraryfolders"];

  for (auto const &partition : vdf_obj.childern) {
    auto apps_iter = partition.second.childern.find("apps");
    if (apps_iter == partition.second.childern.end())
      continue;

    for (auto const &app : apps_iter->second.values) {
      if (app.first == "230410") {
        auto path_iter = partition.second.values.find("path");
        if (path_iter == partition.second.values.end())
          return nullopt;
        full_path = filesystem::path(path_iter->second) / "steamapps" /
                    "common" / "Warframe" / "EE.log";
        if (filesystem::exists(full_path))
          return full_path.string();
        else
          return nullopt;
      }
    }
  }
  return nullopt;
}
#else
optional<string> FindEElogPath() { return ""; }
#endif

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
        DeleteOrder(order.first);
        return;
      }
      case VARS::tradeType::buy: {
        cout << "[Moving] " << slug << endl;

        auto updated_trd = order.second;
        updated_trd.Tstate = VARS::tradeType::sell;
        if (!OrderHandling::PostOrder(updated_trd) ||
            !OrderHandling::DeleteOrder(order.first))
          cout << "[Error] move failed" << endl;
        return;
      }
      }
    } else {
    }
  }
}

void EElogChecking() {
  std::ifstream EE(VARS::Settings.ee_path);
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
      if (!line.empty() && line.back() == '\r')
        line.pop_back();
      if (!line.empty() && line.front() == '\r')
        line.erase(0, 1);

      string element = "";
      stringstream space{line}, comma{line};
      vector<string> spaceSeperated, commaSeperated;

      spaceSeperated = StringOps::SeperateBy(space, element, ' ');
      commaSeperated = StringOps::SeperateBy(comma, element, ',');

      // logic
      if (line.empty())
        continue;

      if (line.find("description=Are you sure you want to accept this trade? "
                    "You are offering:") != std::string::npos) {
        CheckUnlocked = {true, VARS::tradeType::sell};
        soldItems.clear();
        boughtItems.clear();
        continue;
      }

      if (line.find("and will receive from") != std::string::npos) {
        CheckUnlocked.second = VARS::tradeType::buy;
        continue;
      }

      if (commaSeperated.size() > 1 && CheckUnlocked.first) {
        CheckUnlocked = {false, VARS::tradeType::sell};
        std::stringstream ss{*commaSeperated.begin()};
        std::string ln = "";

        boughtItems.push_back(StringOps::SeperateBy(ss, ln, ' '));
        continue;
      }

      if (line.find("SendResult_MENU_CANCEL()") != string::npos) {
        CheckUnlocked = {false, VARS::tradeType::sell};
        soldItems.clear();
        boughtItems.clear();
        continue;
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
        continue;
      }

      if (CheckUnlocked.first && CheckUnlocked.second == VARS::tradeType::sell)
        soldItems.push_back(spaceSeperated);
      else if (CheckUnlocked.first &&
               CheckUnlocked.second == VARS::tradeType::buy)
        boughtItems.push_back(spaceSeperated);
    }

    // wait 334 ms between checks
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
  if ((!validated_trade.has_value() ||
       VARS::Settings.plat - validated_trade->buy < 0) &&
      remote_trade.Tstate == VARS::tradeType::buy) {
    cout << "[Attention] this trade became invalid: " << remote_trade.slug
         << endl;
    OrderHandling::DeleteOrder(order["id"]);
    return;
  }
  if (validated_trade.has_value()) {
    remote_trade.buy = validated_trade->buy;
    remote_trade.sell = validated_trade->sell;

    OrderHandling::UpdateOrder(order["id"], remote_trade);
    return;
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
    OrderHandling::UpdateOrder(iter->first, trd);
  } else {
    OrderHandling::PostOrder(trd);
  }
}

bool DeleteOrder(std::string id) {
  if (id != "") {
    json response = CurlReq::q
                        .Add([id] {
                          return CurlReq::__DELETE(
                              "https://api.warframe.market/v2/order/" + id);
                        })
                        .get();
    if (!response.empty() && response["error"] == nullptr) {
      cout << "[Delete] deleting: "
           << StringOps::GetSlugFromId(response["data"]["itemId"].get<string>())
           << endl;
      Trds::trades.Remove(id);
      return true;
    } else {
      cout << "[Error] removal failed" << endl;
      return false;
    }
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
    for (json const &order : orders["data"]) {
      cout << "[Delete] deleting: "
           << StringOps::GetSlugFromId(order["itemId"].get<string>()) << endl;
      json response = CurlReq::q
                          .Add([order] {
                            return CurlReq::__DELETE(
                                "https://api.warframe.market/v2/order/" +
                                (string)order["id"]);
                          })
                          .get();
    }
    return true;
  }
}

bool UpdateOrder(string trade_id, Trade trd) {
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
  cout << "[Patch] updating: " << trd.slug << endl;
  json response =
      CurlReq::q
          .Add([trade_id, j] {
            return __PATCH("https://api.warframe.market/v2/order/" + trade_id,
                           j.dump());
          })
          .get();

  if (!response.empty() && response["error"] == nullptr) {
    Trds::trades.Set(trade_id, trd);
    return true;
  } else {
    cout << "[Error] update failed" << endl;
    return false;
  }
}

bool PostOrder(Trade trd) {
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
  cout << "[Post] adding: " << trd.slug << endl;
  json response = CurlReq::q
                      .Add([j] {
                        return CurlReq::__POST(
                            "https://api.warframe.market/v2/order", j.dump());
                      })
                      .get();

  if (!response.empty() && response["error"] == nullptr) {
    Trds::trades.Add(response["data"]["id"], trd);
    return true;
  } else {
    cout << "[Error] post failed" << endl;
    return false;
  }
}
} // namespace OrderHandling
