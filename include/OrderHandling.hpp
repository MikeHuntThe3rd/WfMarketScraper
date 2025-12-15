#include <string>
#include <optional>
#include "json.hpp"
#include "Types.hpp"
#include "CurlReq.hpp"

using namespace Types;
using namespace CurlReq;
using json = nlohmann::json;
namespace OrderHandling {
    void PostOrder(std::string JWT, std::string id, trade_return trade);
    void DeleteOrder(std::string JWT, std::string id = "");
}