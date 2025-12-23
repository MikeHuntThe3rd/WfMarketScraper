#include "CurlReq.hpp"
#include "Sorting.hpp"
#include "OrderHandling.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char* argv[]) {
    CurlReq::setup();
    std::cout << "runs" << std::endl;
    if(argc == 1){
	std::cout << "succesfull compalation, no parameter" << std::endl;
    }
    else {
	std::cout << "succesfull compalation, parameter given" << std::endl;
	std::ofstream del("out.log", std::ios::trunc);
	del.close();
	OrderHandling::UpdateOrder(argv[1], "6940730fe2f5699c2150da0d", itemType::mod, tradeType::buy, rank{1,1,1});
	OrderHandling::DeleteOrder(argv[1]);
	json items = CurlReq::GETjson("https://api.warframe.market/v2/items", {"accept: application/json", "Language: en"});
	for(json item: items["data"]){
	    auto trade = Sorting::ValidTrade(item["slug"], item["tags"].get<std::vector<std::string>>(), true);
	    if(trade.good_trade){
		std::string id = item["id"].get<std::string>();
		OrderHandling::PostOrder(argv[1], id, tradeType::buy, trade);
	    }
	}
	CurlReq::disconnect();
    }
    return 0;
}
