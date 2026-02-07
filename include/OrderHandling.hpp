#pragma once
#include <cstring>
#include <string>
#include <sstream>
#include <optional>
#include <vector>
#include <fstream>
#include "json.hpp"
#include "VARS.hpp"
#include "CurlReq.hpp"

using namespace VARS;
using namespace CurlReq;
using json = nlohmann::json;
namespace OrderHandling {
    void PostOrder(std::string JWT, std::string id, tradeType type, trade_return trade);
    void DeleteOrder(std::string JWT, std::string id = "");
    void UpdateOrder(std::string JWT, std::string id, itemType type, tradeType trade_type, std::any data);
    std::vector<std::string> SeperateBy(std::stringstream & SS, std::string & line, char separator); 
    void EElogChecking();
    void HandleTrades(std::vector<local_trade> Trades, std::string JWT);
    void HandleSell(local_trade & trade, json & orders);
    void HandleBuy(local_trade & trade, json & orders);
    local_trade ResultConversion(std::vector<std::string> item, tradeType Ttype);
    std::string Implode(std::vector<std::string> array, char seperator);
    std::string GetIdFromSlug(std::string slug);
    VARS::itemType GetITypeFromSlug(std::string slug);
    void LowerCase(std::string & word);
}
