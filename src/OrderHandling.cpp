#include "OrderHandling.hpp"
namespace OrderHandling {
    void PostOrder(std::string id, std::string JWT, trade_return trade){
        json j;
        j["itemId"] = id;
        j["type"] = "buy";
        j["quantity"] = 1;
        j["visible"] = false;
        j["perTrade"] = 1;
        
        switch (trade.type)
        {
        case itemType::basic : {
            j["platinum"] = std::any_cast<basic>(trade.data).buy;
            POSTjson("https://api.warframe.market/v2/order", JWT, j);
            break;
        }
        case itemType::mod : {
            j["platinum"] = std::any_cast<rank>(trade.data).buy;
            j["rank"] = std::any_cast<rank>(trade.data).level;
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