#include "main.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <thread>
#include <vector>

vector<string> ParseArgv(int argc, char *argv[]) {
  vector<string> result(argv, argv + argc);
  return result;
}

void ChangeSettings(optional<MapElement> change) {
  // set change if given
  if (change.has_value())
    VARS::Settings[change->key] = change->value;
  // remake the entire json with the settings unordered map as a template
  using json = nlohmann::json;
  json def;
  for (auto element : VARS::Settings) {
    def[element.first] = element.second;
  }
  ofstream DefSettings("settings.json");
  DefSettings << def;
  DefSettings.close();
}

void CreateSettings() {
  string fileName = "settings.json";
  ifstream file(fileName.c_str());
  if (!file.good()) {
    cout << "settings generated with default settings" << endl;
    ChangeSettings();
  }
}

void SettingsSetup() {
  using json = nlohmann::json;
  CreateSettings();
  ifstream in("settings.json");
  json data = json::parse(in);
  for (const auto &[key, value] : data.items()) {
    auto setting = VARS::Settings.find(key);
    if (setting != VARS::Settings.end()) {
      try {
        setting->second = value;
      } catch (exception e) {
        cout << e.what() << endl;
      }
    }
  }
}

void WebLoop() {
  CurlReq::setup();
  ofstream del("out.log", ios::trunc);
  del.close();
  // OrderHandling::DeleteOrder();
  items = CurlReq::q
              .Add([] {
                return CurlReq::__GET("https://api.warframe.market/v2/items");
              })
              .get();
  while (VARS::LogLoop) {
    for (json item : items["data"]) {
      auto trade = Sorting::ValidTrade(item["slug"], item["tags"],
                                       VARS::Settings["-l"] != 0);
      if (trade.has_value())
        OrderHandling::HandleNewTrade(trade.value());
    }
  }
  CurlReq::disconnect();
}

void Set(vector<string> args) {
  if (args.size() > 4) {
    cout << "too many arguments given" << endl;
    return;
  }
  optional<string> mode =
      (args.size() >= 3) ? optional<string>{(string)args[2]}
                         : nullopt;
  if (mode.has_value()) {
    CreateSettings();
    auto setting = VARS::Settings.find(mode.value());
    int value;
    try {
      if (args.size() < 4) {
        cout << "no value given" << endl;
        return;
      }
      value = stoi(args[3]);
    } catch (exception e) {
      cout << "error: " << e.what() << endl;
      return;
    }
    if (setting != VARS::Settings.end()) {
      SettingsSetup();
      ChangeSettings(MapElement{setting->first, value});
      cout << "setting changed succesfully" << endl;
    } else {
      cout << "set mode not found" << endl;
      return;
    }

  } else {
    cout << "no set mode given" << endl;
  }
}

void Run(vector<string> args) {
  if (args.size() > 4) {
    cout << "too many arguments given" << endl;
    return;
  }
  if (args.size() == 4) {
    string mode = args[2];
    if (VARS::RunModes.find(mode) == VARS::RunModes.end()) {
      cout << "mode not recognized" << endl;
      return;
    }

    VARS::JWT = args[3];

    if (mode == "-w") {
      SettingsSetup();
      WebLoop();
    } else if (mode == "-d") {
      CurlReq::setup();
      OrderHandling::DeleteOrder();
      CurlReq::q.WaitUntilQueueEmpty();
      CurlReq::disconnect();
    } else if (mode == "-l") {
      CurlReq::setup();
      items =
          CurlReq::q
              .Add([] {
                return CurlReq::__GET("https://api.warframe.market/v2/items");
              })
              .get();
      OrderHandling::EElogChecking();
      CurlReq::disconnect();
    }
  }
  if (args.size() == 3) {
    if (VARS::RunModes.find(args[2]) != VARS::RunModes.end()) {
      cout << "invalid command. \nNOTE:all web operation require your jwt "
                   "token in the last argument"
                << endl;
      return;
    }
    VARS::JWT = args[2];
    thread InGameTrades(OrderHandling::EElogChecking);
    WebLoop();
    InGameTrades.join();
  }
}

void CommandDispatch(vector<string> args) {
  if (args.size() <= 2) {
    cout << "too few arguments" << endl;
    return;
  }
  if ((string)args[1] == "run") {
    Run(args);
  } else if ((string)args[1] == "set") {
    Set(args);
  } else {
    cout << "command not recognized" << endl;
  }
}

int main(int argc, char *argv[]) {
  CommandDispatch(ParseArgv(argc, argv));
  return 0;
}
