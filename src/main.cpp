#include "main.hpp"
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

void LoadSettings(){
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
      auto trade = Sorting::ValidTrade(item["slug"], item["tags"],
                                       VARS::Settings.log);
      if (trade.has_value())
        OrderHandling::HandleNewTrade(trade.value());
    }
  }
  CurlReq::disconnect();
}

// void Set(vector<string> args) {
//   if (args.size() > 4) {
//     cout << "too many arguments given" << endl;
//     return;
//   }
//   optional<string> mode =
//       (args.size() >= 3) ? optional<string>{(string)args[2]}
//                          : nullopt;
//   if (mode.has_value()) {
//     CreateSettings();
//     auto setting = VARS::Settings.find(mode.value());
//     int value;
//     try {
//       if (args.size() < 4) {
//         cout << "no value given" << endl;
//         return;
//       }
//       value = stoi(args[3]);
//     } catch (exception e) {
//       cout << "error: " << e.what() << endl;
//       return;
//     }
//     if (setting != VARS::Settings.end()) {
//       SettingsSetup();
//       ChangeSettings(MapElement{setting->first, value});
//       cout << "setting changed succesfully" << endl;
//     } else {
//       cout << "set mode not found" << endl;
//       return;
//     }

//   } else {
//     cout << "no set mode given" << endl;
//   }
// }

// void Run(vector<string> args) {
//   if (args.size() > 4) {
//     cout << "too many arguments given" << endl;
//     return;
//   }
//   if (args.size() == 4) {
//     string mode = args[2];
//     if (VARS::RunModes.find(mode) == VARS::RunModes.end()) {
//       cout << "mode not recognized" << endl;
//       return;
//     }

//     VARS::JWT = args[3];

//     if (mode == "-w") {
//       WebLoop();
//     } else if (mode == "-d") {
//       CurlReq::setup();
//       OrderHandling::DeleteOrder();
//       CurlReq::q.WaitUntilQueueEmpty();
//       CurlReq::disconnect();
//     } else if (mode == "-l") {
//       CurlReq::setup();
//       items =
//           CurlReq::q
//               .Add([] {
//                 return CurlReq::__GET("https://api.warframe.market/v2/items");
//               })
//               .get();
//       OrderHandling::EElogChecking();
//       CurlReq::disconnect();
//     }
//   }
//   if (args.size() == 3) {
//     if (VARS::RunModes.find(args[2]) != VARS::RunModes.end()) {
//       cout << "invalid command. \nNOTE:all web operation require your jwt "
//                    "token in the last argument"
//                 << endl;
//       return;
//     }
//     VARS::JWT = args[2];
//     thread InGameTrades(OrderHandling::EElogChecking);
//     WebLoop();
//     InGameTrades.join();
//   }
// }

void Run(CLI::App* run, optional<string> jwt){
  CreateSettings();
  LoadSettings();

  if (run->got_subcommand("web"))
  {
    if(!jwt.has_value() && VARS::Settings.JWT.size() == 0) throw VARS::costum_exit{1, "jwt isnt saved or given as a parameter"};
    else if(jwt.has_value() && VARS::Settings.JWT.size() == 0) VARS::JWT = jwt.value();
    else VARS::JWT = VARS::Settings.JWT;

    WebLoop();
  }
  else throw VARS::costum_exit{1, "i didnt write that yet nigga"};
  
}

int main(int argc, char *argv[]) {
  try{
    CLI::App app{"Warframe Market Scraper"};
    optional<string> jwt;
    auto run = app.add_subcommand("run", "subcommand for starting the scraper");
    run->add_option("-j,--jwt", jwt, "your personal jwt token")->expected(0, 1);
    run->add_subcommand("web", "scrapes the warframe market site for benefitial trades");
    run->add_subcommand("delete", "deletes all trades");
    run->add_subcommand("local", "runs local file checking");

    auto set = app.add_subcommand("set", "subcommand for manadging settings");

    if(app.got_subcommand(run)) Run(run, jwt);
  }
  catch(const VARS::costum_exit& e){
    cerr << "[Exit] " << e.msg << endl;
    return e.code;
  }
  catch (const exception& e) {
      cerr << "[Fatal] " << e.what() << endl;
      return 1;
  }
  return 0;
}
