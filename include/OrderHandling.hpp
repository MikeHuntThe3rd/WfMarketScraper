#include <string>
#include <sstream>
#include <optional>
#include <vector>
#include <fstream>
#include "json.hpp"
#include "Types.hpp"
#include "CurlReq.hpp"

using namespace Types;
using namespace CurlReq;
using json = nlohmann::json;
namespace OrderHandling {
    void PostOrder(std::string JWT, std::string id, tradeType type, trade_return trade);
    void DeleteOrder(std::string JWT, std::string id = "");
    void UpdateOrder(std::string JWT, std::string id, itemType type, tradeType trade_type, std::any data);
    std::vector<std::string> SeperateBy(std::stringstream & SS, std::string & line, char separator); 
    void EElogChecking();
    local_trade ResultConversion(std::vector<std::string> item, tradeType Ttype);
    std::string Implode(std::vector<std::string> array, char seperator);
}
