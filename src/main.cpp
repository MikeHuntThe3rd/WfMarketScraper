#include <atomic>
#include <fstream>
#include <iostream>
#include <thread>
#include "CurlReq.hpp"
#include "Sorting.hpp"
#include "OrderHandling.hpp"

int main(int argc, char* argv[]) {
    CurlReq::setup();
    std::cout << "runs" << std::endl;
    if(argc == 1){
		std::cout << "succesfull compalation, no parameter" << std::endl;
		std::thread InGameTrades(OrderHandling::EElogChecking);
		InGameTrades.join();
    }
    else {
	std::ofstream del("out.log", std::ios::trunc);
	del.close();
	OrderHandling::DeleteOrder(argv[1]);
	json items = CurlReq::q.Add([]{ return CurlReq::__GET("https://api.warframe.market/v2/items");}).get();
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
