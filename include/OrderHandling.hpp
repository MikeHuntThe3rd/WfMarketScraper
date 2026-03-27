#pragma once
#include "CurlReq.hpp"
#include "VARS.hpp"
#include "json.hpp"
#include <cstring>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

using namespace VARS;
using namespace CurlReq;
using json = nlohmann::json;

namespace OrderHandling {

// functions
// Major functions
void EElogChecking();
// Helper functions
void PostOrder(Trade trd);
void DeleteOrder(std::string id = "");
void UpdateOrder(Trade trd);
std::vector<std::string> SeperateBy(std::stringstream &SS, std::string &line,
                                    char separator);
void HandleTrades(std::vector<Trade> Trades);
void HandleSell(Trade &trade, json &orders);
void HandleBuy(Trade &trade, json &orders);
Trade ResultConversion(std::vector<std::string> item, tradeType Ttype);
std::string Implode(std::vector<std::string> array, char seperator);
std::string GetIdFromSlug(std::string slug);
VARS::itemType GetITypeFromSlug(std::string slug);
void LowerCase(std::string &word);
} // namespace OrderHandling
