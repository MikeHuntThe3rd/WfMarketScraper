#pragma once
#include "CurlReq.hpp"
#include "VARS.hpp"
#include "json.hpp"
#include <cstring>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace VARS;
using namespace CurlReq;
using json = nlohmann::json;
namespace OrderHandling {
// Trades
class Trades {
public:
  Trades();
  std::vector<VARS::Trade> Read();
  void Add(Trade trade);
  void Remove(std::string id);
  void Edit(std::string id, Trade trade);

private:
  void Manager();
  std::vector<VARS::Trade> trades;
  std::mutex thread_lock;
  std::condition_variable cv;
  std::thread managerT;
};
inline Trades trades{};
// functions
// Major functions
void EElogChecking();
// Helper functions
void PostOrder(std::string id, tradeType type, Trade trade);
void DeleteOrder(std::string id = "");
void UpdateOrder(std::string id, itemType type, tradeType trade_type,
                 std::any data);
std::vector<std::string> SeperateBy(std::stringstream &SS, std::string &line,
                                    char separator);
void HandleTrades(std::vector<local_trade> Trades);
void HandleSell(local_trade &trade, json &orders);
void HandleBuy(local_trade &trade, json &orders);
local_trade ResultConversion(std::vector<std::string> item, tradeType Ttype);
std::string Implode(std::vector<std::string> array, char seperator);
std::string GetIdFromSlug(std::string slug);
VARS::itemType GetITypeFromSlug(std::string slug);
void LowerCase(std::string &word);
} // namespace OrderHandling
