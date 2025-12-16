#include "OrderHandling.hpp"
namespace OrderHandling {
    void DeleteOrder(std::string JWT, std::string id){
        if(id != ""){
            CurlReq::__DELETE("https://api.warframe.market/v2/order/" + id, JWT);
        }
        else {
            json orders = CurlReq::GETjson("https://api.warframe.market/v2/orders/my", {"Content-Type: application/json", "Accept: application/json", "Authorization: Bearer " + JWT});
            for(json order: orders["data"]){
                CurlReq::__DELETE("https://api.warframe.market/v2/order/" + order["id"].get<std::string>(), JWT);
            }
        }
    }
    void UpdateOrder(std::string JWT, std::string id, itemType type, std::optional<std::any> data){
        json j;
        j["quantity"] = 1;
        j["visible"] = false;
        
        switch (type)
        {
            case itemType::basic : {
                j["platinum"] = std::any_cast<basic>(data).buy;
                __PATCH("https://api.warframe.market/v2/order" + id, JWT, j.dump());
                break;
            }
            case itemType::mod : {
                j["platinum"] = std::any_cast<rank>(data).buy;
                j["rank"] = std::any_cast<rank>(data).level;
                __PATCH("https://api.warframe.market/v2/order" + id, JWT, j.dump());
                break;
            }
            case itemType::Ayatan : {
                j["perTrade"] = 1;
                __PATCH("https://api.warframe.market/v2/order" + id, JWT, j.dump());
                break;
            }
        }
    }
    void PostOrder(std::string JWT, std::string id, tradeType type, trade_return trade){
        json j;
        j["itemId"] = id;
        if(type == tradeType::buy) j["type"] = "buy";
        else j["type"] = "sell";
        j["quantity"] = 1;
        j["visible"] = false;
        // j["perTrade"] = 1;
        
        switch (trade.type)
        {
            case itemType::basic : {
                if(type == tradeType::buy) j["platinum"] = std::any_cast<basic>(trade.data).buy;
                else j["platinum"] = std::any_cast<basic>(trade.data).sell;
                POSTjson("https://api.warframe.market/v2/order", JWT, j.dump());
                break;
            }
            case itemType::mod : {
                if(type == tradeType::buy) j["platinum"] = std::any_cast<rank>(trade.data).buy;
                else j["platinum"] = std::any_cast<rank>(trade.data).sell;
                j["rank"] = std::any_cast<rank>(trade.data).level;
                POSTjson("https://api.warframe.market/v2/order", JWT, j.dump());
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