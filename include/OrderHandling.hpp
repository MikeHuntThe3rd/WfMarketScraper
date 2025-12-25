#include <string>
#include <sstream>
#include <optional>
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
    void EElogChecking();
    
}
