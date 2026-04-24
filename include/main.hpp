#include "CLI11.hpp"
#include "CurlReq.hpp"
#include "OrderHandling.hpp"
#include "Sorting.hpp"
#include "VARS.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

// this is commented out intentionally its only in the source
//  template<typename T>
//  void AddCommandTriage(CLI::App* set, string name, string help);

void CreateSettings();
void LoadSettings();
void Initialize();
void WebLoop();
void Run(CLI::App *run, optional<string> jwt);
void Set(CLI::App *set);
