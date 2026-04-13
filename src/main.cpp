#include "main.hpp"
#include "CLI11.hpp"
#include "VARS.hpp"
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

template <typename T>
void AddCommandtriplex(CLI::App *set, string name, string help) {
  auto cmd = set->add_subcommand(name, help);
  auto reset = make_shared<bool>(false);
  auto show = make_shared<bool>(false);
  auto val = make_shared<optional<T>>();

  cmd->add_flag("-r,--reset", *reset, "resets the associated element");
  cmd->add_flag("-s,--show", *show, "shows the associated element");
  cmd->add_option("val", *val, "sets the value for the element");

  cmd->callback([name, reset, show, val]() {
    CreateSettings();
    ifstream ifs("settings.json");
    json setts = json::parse(ifs);

    if (*show) {
      cout << name << " value: " << setts[name] << endl;
    } else if (*reset) {
      cout << "resetting: " << name << endl;
      setts[name] = VARS::DefSettings[name];
      ofstream set_file("settings.json", ios::trunc);
      set_file << setts.dump(4);
    } else if (val->has_value()) {
      cout << "setting: " << name << " to: " << boolalpha << val->value()
           << noboolalpha << endl;
      setts[name] = val->value();
      ofstream set_file("settings.json", ios::trunc);
      set_file << setts.dump(4);
    } else {
      throw VARS::costum_exit{1, "idk what triggers this yet"};
    }
  });
}

void CreateSettings() {
  if (!filesystem::exists("settings.json")) {
    cout << "settings.json generated with default settings" << endl;
    ofstream settings_gen("settings.json");
    settings_gen << VARS::DefSettings.dump(4);
  }
}

void LoadSettings() {
  json loaded_settings;
  ifstream file("settings.json");
  loaded_settings = json::parse(file);
  VARS::Settings = loaded_settings.get<VARS::Settings_template>();
}

void WebLoop() {
  CurlReq::setup();
  ofstream del("out.log", ios::trunc);
  del.close();
  cout << "[Attention] current balance: " << VARS::Settings.plat << endl;
  if (VARS::Settings.dos)
    OrderHandling::DeleteOrder();
  items = CurlReq::q
              .Add([] {
                return CurlReq::__GET("https://api.warframe.market/v2/items");
              })
              .get();
  while (VARS::thread_run) {
    for (json item : items["data"]) {
      auto trade =
          Sorting::ValidTrade(item["slug"], item["tags"], VARS::Settings.log);
      if (trade.has_value() && VARS::Settings.plat - trade.value().buy >= 0)
        OrderHandling::HandleNewTrade(trade.value());
      else if (trade.has_value() && VARS::Settings.plat - trade.value().buy < 0)
        cout << "[Attention] balance is lower than buy price for: "
             << trade.value().slug << endl;
      ;
    }
  }
  CurlReq::disconnect();
}

void Run(CLI::App *run, optional<string> jwt) {
  CreateSettings();
  LoadSettings();
  if (!jwt.has_value() && VARS::Settings.jwt.size() == 0)
    throw VARS::costum_exit{1, "jwt isnt saved or given as a parameter"};
  else if (jwt.has_value() && VARS::Settings.jwt.size() == 0) {
    cout << "using JWT from cli" << endl
         << "JWT will not be saved anywhere" << endl;
    VARS::JWT = jwt.value();
  } else
    VARS::JWT = VARS::Settings.jwt;

  if (run->got_subcommand("web")) {
    WebLoop();
  } else if (run->got_subcommand("delete")) {
    CurlReq::setup();
    OrderHandling::DeleteOrder();
    CurlReq::q.WaitUntilQueueEmpty();
    CurlReq::disconnect();
  } else if (run->got_subcommand("local")) {
    CurlReq::setup();
    items = CurlReq::q
                .Add([] {
                  return CurlReq::__GET("https://api.warframe.market/v2/items");
                })
                .get();
    OrderHandling::EElogChecking();
    CurlReq::disconnect();
  } else {
    std::thread InGameTrades(OrderHandling::EElogChecking);
    WebLoop();
    InGameTrades.join();
  }
}

void Set(CLI::App *set) {
  CreateSettings();
  if (set->got_subcommand("reset_all")) {
    ofstream of("settings.json", ios::trunc);
    of << VARS::DefSettings.dump(4);
  }
  if (set->got_subcommand("show_all")) {
    ifstream ifs("settings.json");
    json loaded_setts = json::parse(ifs);

    for (const auto &[key, val] : loaded_setts.items()) {
      cout << key << ": " << boolalpha << val << noboolalpha << endl;
    }
  }
}

int main(int argc, char *argv[]) {
  try {
    CLI::App app{"Warframe Market Scraper"};
    optional<string> jwt;
    auto run = app.add_subcommand("run", "subcommand for starting the scraper");
    run->add_option("-j,--jwt", jwt, "your personal jwt token")->expected(0, 1);
    run->add_subcommand(
        "web", "scrapes the warframe market site for benefitial trades");
    run->add_subcommand("delete", "deletes all trades");
    run->add_subcommand("local", "runs local file checking");

    auto set = app.add_subcommand("set", "subcommand for managing settings");

    auto res_all = set->add_subcommand(
        "reset_all", "resets all values to their default states");
    auto show_all = set->add_subcommand("show_all", "shows all values");
    AddCommandtriplex<string>(set, "jwt", "your personal jwt token");
    AddCommandtriplex<long long int>(
        set, "margin", "the minimum margin amount required for a good trade");
    AddCommandtriplex<long long int>(
        set, "freq", "the minimum trade frequency required for a good trade");
    AddCommandtriplex<long long int>(set, "plat", "your current known balance");
    AddCommandtriplex<bool>(set, "log", "enable/disable logging");
    AddCommandtriplex<bool>(
        set, "dos",
        "enable/disable deleting all sales at the start of a web operation");
    AddCommandtriplex<bool>(
        set, "vt",
        "enable/disable whether trades found are posted as visible or not");

    app.add_option_function<int>(
           "-v,--version",
           [](int useless) { cout << "WfMarketScraper version 1.0.0" << endl; },
           "Print the current version and exit")
        ->expected(0)
        ->type_name("");
    CLI11_PARSE(app, argc, argv)

    if (app.got_subcommand(run))
      Run(run, jwt);
    if (app.got_subcommand(set))
      Set(set);
  } catch (const VARS::costum_exit &e) {
    cerr << "[Exit] " << e.msg << endl;
    return e.code;
  } catch (const exception &e) {
    cerr << "[Fatal] " << e.what() << endl;
    return 1;
  }
  return 0;
}
