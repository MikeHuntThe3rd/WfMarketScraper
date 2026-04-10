#include "VARS.hpp"
#include "CurlReq.hpp"
#include "OrderHandling.hpp"
#include "Sorting.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

struct MapElement {
  std::string key;
  int value;
};

vector<string> ParseArgv(int argc, char *argv[]);
void ChangeSettings(optional<MapElement> change = nullopt);
void CreateSettings();
void SettingsSetup();
void WebLoop();
void Set(vector<string> args);
void Run(vector<string> args);
void CommandDispatch(vector<string> args);