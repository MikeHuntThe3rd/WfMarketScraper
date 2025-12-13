#include "OrderHandling.hpp"
namespace OrderHandling {
    void PostOrder(itemType type, std::string id, std::string JWT, std::optional<std::any> data){
        json j;
        j["itemId"] = id;
        j["type"] = "buy";
        j["quantity"] = 1;
        j["visible"] = "false";
        
        switch (type)
        {
        case itemType::basic : {
            // j["platinum"] = std::any_cast<basic>(data.value()).buy;
            POSTjson("https://api.warframe.market/v2/order", JWT, j);
            break;
        }
        case itemType::mod : {
            // j["platinum"] = std::any_cast<rank>(data.value()).buy;
            // j["rank"] = std::any_cast<rank>(data.value()).level;
            POSTjson("https://api.warframe.market/v2/order", JWT, j);
            break;
        }
        case itemType::Ayatan : {
            // std::string bdy = R"({
            //     "perTrade": 1
            // })";
            break;
        }
        }
    }
}