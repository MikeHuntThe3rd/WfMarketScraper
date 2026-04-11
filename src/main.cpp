#include "main.hpp"
#include "CLI11.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <thread>
#include <vector>

void CreateSettings() {
  ifstream file("settings.json");
  if (!file.good()) {
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
  items = CurlReq::q
              .Add([] {
                return CurlReq::__GET("https://api.warframe.market/v2/items");
              })
              .get();
  while (VARS::thread_run) {
    for (json item : items["data"]) {
      auto trade =
          Sorting::ValidTrade(item["slug"], item["tags"], VARS::Settings.log);
      if (trade.has_value())
        OrderHandling::HandleNewTrade(trade.value());
    }
  }
  CurlReq::disconnect();
}

void Run(CLI::App *run, optional<string> jwt) {
  CreateSettings();
  LoadSettings();
  if (!jwt.has_value() && VARS::Settings.JWT.size() == 0)
    throw VARS::costum_exit{1, "jwt isnt saved or given as a parameter"};
  else if (jwt.has_value() && VARS::Settings.JWT.size() == 0){
    cout << "using JWT from cli" << endl <<"JWT will not be saved anywhere" << endl;
    VARS::JWT = jwt.value();
  }
  else
    VARS::JWT = VARS::Settings.JWT;

  if (run->got_subcommand("web")) {
    WebLoop();
  } else if(run->got_subcommand("delete")){
    CurlReq::setup();
    OrderHandling::DeleteOrder();
    CurlReq::q.WaitUntilQueueEmpty();
    CurlReq::disconnect();
  }else if(run->got_subcommand("local")){
    CurlReq::setup();
  cout << "hewwo" << endl;
    items =
        CurlReq::q
            .Add([] {
              return CurlReq::__GET("https://api.warframe.market/v2/items");
            })
            .get();
  cout << "hewwo" << endl;
    OrderHandling::EElogChecking();
    CurlReq::disconnect();
  }else{
    std::thread InGameTrades(OrderHandling::EElogChecking);
    WebLoop();
    InGameTrades.join();
  }
}

void Set(CLI::App* set){

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

    auto set = app.add_subcommand("set", "subcommand for manadging settings");
    CLI11_PARSE(app, argc, argv)

    if (app.got_subcommand(run))
      Run(run, jwt);
  } catch (const VARS::costum_exit &e) {
    cerr << "[Exit] " << e.msg << endl;
    return e.code;
  } catch (const exception &e) {
    cerr << "[Fatal] " << e.what() << endl;
    return 1;
  }
  return 0;
}
