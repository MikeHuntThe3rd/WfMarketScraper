#pragma once
#include "CurlReq.hpp"
#include "VARS.hpp"
#include "json.hpp"
#include <sstream>
#include <string>
#include <vector>

using namespace VARS;
using namespace CurlReq;
using namespace std;
using json = nlohmann::json;

namespace OrderHandling {

// Major functions(should be in its own thread)
void EElogChecking();
// Helper functions
json PostOrder(Trade trd);
json DeleteOrder(std::string id = "");
json UpdateOrder(string trade_id, Trade trd);
std::vector<std::string> SeperateBy(std::stringstream &SS, std::string &line,
                                    char separator);

// trade manager functions
void HandleNewTrade(Trade trd);
void HandleTradeMove(vector<string> item, tradeType Ttype);

// generic functions
Trade ResultConversion(std::vector<std::string> item, tradeType Ttype);
std::string Implode(std::vector<std::string> array, char seperator);
std::string GetIdFromSlug(std::string slug);
VARS::itemType GetITypeFromSlug(std::string slug);
void LowerCase(std::string &word);
} // namespace OrderHandling
