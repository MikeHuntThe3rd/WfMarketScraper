#include <string>
#include <optional>
#include "json.hpp"
#include "Types.hpp"
#include "CurlReq.hpp"

using namespace Types;
using namespace CurlReq;
using json = nlohmann::json;
namespace OrderHandling {
    void PostOrder(itemType type, std::string id, std::string JWT, std::optional<std::any> data);
}