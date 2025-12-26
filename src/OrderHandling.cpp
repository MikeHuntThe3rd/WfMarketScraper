#include "OrderHandling.hpp"
#include "CurlReq.hpp"
#include "Types.hpp"

namespace OrderHandling {
    void EElogChecking(){
	std::ifstream EE("../../out.txt");
	std::string line;
	while(Types::LogLoop){
	    EE.clear();
	    EE.seekg(0, std::ios::beg);
	    while (std::getline(EE, line)) {
            if(line.find("description=Are you sure you want to accept this trade? You are offering:") != std::string::npos){
                std::cout << line << std::endl;
            }
	    }
        CurlReq::wait();
    }	
    }
    void DeleteOrder(std::string JWT, std::string id){
        if(id != ""){
            CurlReq::q.Add([id, JWT]{return CurlReq::__DELETE("https://api.warframe.market/v2/order/" + id, JWT);});
        }
        else {
            json orders = CurlReq::q.Add([JWT]{ return CurlReq::__GET("https://api.warframe.market/v2/orders/my", {"Content-Type: application/json", "Accept: application/json", "Authorization: Bearer " + JWT});}).get();
            for(json order: orders["data"]){
                CurlReq::q.Add([order, JWT]{ return CurlReq::__DELETE("https://api.warframe.market/v2/order/" + order["id"].get<std::string>(), JWT);});
            }
        }
    }
    void UpdateOrder(std::string JWT, std::string id, itemType type, tradeType trade_type, std::any data){
        json j;
        j["quantity"] = 1;
        j["visible"] = false;
        
        switch (type)
        {
            case itemType::basic : {
                if(trade_type == tradeType::buy)j["platinum"] = std::any_cast<basic>(data).buy;
                else j["platinum"] = std::any_cast<basic>(data).sell;
                break;
            }
            case itemType::mod : {
                if(trade_type == tradeType::buy) j["platinum"] = std::any_cast<rank>(data).buy;
                else j["platinum"] = std::any_cast<rank>(data).sell;
                j["rank"] = std::any_cast<rank>(data).level;
                break;
            }
            case itemType::Ayatan : {
                j["perTrade"] = 1;
                break;
            }
        }

        CurlReq::q.Add([id, JWT, j]{ return __PATCH("https://api.warframe.market/v2/order/" + id, JWT, j.dump());});
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
                break;
            }
            case itemType::mod : {
                if(type == tradeType::buy) j["platinum"] = std::any_cast<rank>(trade.data).buy;
                else j["platinum"] = std::any_cast<rank>(trade.data).sell;
                j["rank"] = std::any_cast<rank>(trade.data).level;
                break;
            }
            case itemType::Ayatan : {
                // std::string bdy = R"({
                //     "perTrade": 1
                // })";
                break;
            }
        }
        CurlReq::q.Add([JWT, j]{ return CurlReq::__POST("https://api.warframe.market/v2/order", JWT, j.dump());});
    }
}
