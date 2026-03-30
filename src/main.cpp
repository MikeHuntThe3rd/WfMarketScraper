#include "CurlReq.hpp"
#include "OrderHandling.hpp"
#include "Sorting.hpp"
#include "VARS.hpp"
#include "json.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <thread>
#include <vector>

struct MapElement {
  std::string key;
  int value;
};

std::vector<std::string> ParseArgv(int argc, char *argv[]) {
  std::vector<std::string> result(argv, argv + argc);
  return result;
}

void ChangeSettings(std::optional<MapElement> change = std::nullopt) {
  // set change if given
  if (change.has_value())
    VARS::Settings[change->key] = change->value;
  // remake the entire json with the settings unordered map as a template
  using json = nlohmann::json;
  json def;
  for (auto element : VARS::Settings) {
    def[element.first] = element.second;
  }
  std::ofstream DefSettings("settings.json");
  DefSettings << def;
  DefSettings.close();
}

void CreateSettings() {
  std::string fileName = "settings.json";
  std::ifstream file(fileName.c_str());
  if (!file.good()) {
    std::cout << "settings generated with default settings" << std::endl;
    ChangeSettings();
  }
}

void SettingsSetup() {
  using json = nlohmann::json;
  CreateSettings();
  std::ifstream in("settings.json");
  json data = json::parse(in);
  for (const auto &[key, value] : data.items()) {
    auto setting = VARS::Settings.find(key);
    if (setting != VARS::Settings.end()) {
      try {
        setting->second = value;
      } catch (std::exception e) {
        std::cout << e.what() << std::endl;
      }
    }
  }
}

void WebLoop() {
  CurlReq::setup();
  std::ofstream del("out.log", std::ios::trunc);
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

void Set(std::vector<std::string> args) {
  if (args.size() > 4) {
    std::cout << "too many arguments given" << std::endl;
    return;
  }
  std::optional<std::string> mode =
      (args.size() >= 3) ? std::optional<std::string>{(std::string)args[2]}
                         : std::nullopt;
  if (mode.has_value()) {
    CreateSettings();
    auto setting = VARS::Settings.find(mode.value());
    int value;
    try {
      if (args.size() < 4) {
        std::cout << "no value given" << std::endl;
        return;
      }
      value = std::stoi(args[3]);
    } catch (std::exception e) {
      std::cout << "error: " << e.what() << std::endl;
      return;
    }
    if (setting != VARS::Settings.end()) {
      SettingsSetup();
      ChangeSettings(MapElement{setting->first, value});
      std::cout << "setting changed succesfully" << std::endl;
    } else {
      std::cout << "set mode not found" << std::endl;
      return;
    }

  } else {
    std::cout << "no set mode given" << std::endl;
  }
}

void Run(std::vector<std::string> args) {
  if (args.size() > 4) {
    std::cout << "too many arguments given" << std::endl;
    return;
  }
  if (args.size() == 4) {
    std::string mode = args[2];
    if (VARS::RunModes.find(mode) == VARS::RunModes.end()) {
      std::cout << "mode not recognized" << std::endl;
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
      std::cout << "invalid command. \nNOTE:all web operation require your jwt "
                   "token in the last argument"
                << std::endl;
      return;
    }
    VARS::JWT = args[2];
    std::thread InGameTrades(OrderHandling::EElogChecking);
    WebLoop();
    InGameTrades.join();
  }
}

void CommandDispatch(std::vector<std::string> args) {
  if (args.size() <= 2) {
    std::cout << "too few arguments" << std::endl;
    return;
  }
  if ((std::string)args[1] == "run") {
    Run(args);
  } else if ((std::string)args[1] == "set") {
    Set(args);
  } else {
    std::cout << "command not recognized" << std::endl;
  }
}

int main(int argc, char *argv[]) {
  CommandDispatch(ParseArgv(argc, argv));
  return 0;
}
