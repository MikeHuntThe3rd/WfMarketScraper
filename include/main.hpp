#include "VARS.hpp"
#include "CurlReq.hpp"
#include "OrderHandling.hpp"
#include "Sorting.hpp"
#include "CLI11.hpp"
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

void CreateSettings();
void LoadSettings();
void WebLoop();
void Set(vector<string> args);
void Run(CLI::App* run, optional<string> jwt);