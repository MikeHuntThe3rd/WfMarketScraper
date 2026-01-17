#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <thread>
#include <vector>
#include "CurlReq.hpp"
#include "Sorting.hpp"
#include "OrderHandling.hpp"
#include "Types.hpp"
#include "optionparser.h"

void WebLoop(std::string jwt){
    CurlReq::setup();
    std::ofstream del("out.log", std::ios::trunc);
    del.close();
    OrderHandling::DeleteOrder(jwt);
    json items = CurlReq::q.Add([]{ return CurlReq::__GET("https://api.warframe.market/v2/items");}).get();
    while (Types::LogLoop) {
	for(json item: items["data"]){
	    auto trade = Sorting::ValidTrade(item["slug"], item["tags"].get<std::vector<std::string>>(), true);
	    if(trade.good_trade){
		std::string id = item["id"].get<std::string>();
		OrderHandling::PostOrder(jwt, id, tradeType::buy, trade);
	    }
	}
    }
    CurlReq::disconnect();
}

std::vector<std::string> ParseArgv(int argc, char* argv[]){
    std::vector<std::string> result(argv, argv + argc);
    return result;
}

void Run(std::vector<std::string> args){
    if(args.size() > 4){
	std::cout << "too many arguments given" << std::endl;
	return;
    }
    std::optional<std::string> mode = (args.size() >= 3) ? std::optional<std::string>{(std::string)args[2]} : std::nullopt;
    if(mode.has_value()){
	if(Types::RunModes.find(mode.value()) == Types::RunModes.end()){
	    std::cout << "mode not recognized" << std::endl;
	    return;
	}
	if(mode.value() == "-w" && args.size() > 3){
	    WebLoop(args[3]);
	}
	else if (mode.value() == "-d" && args.size() > 3) {
	    CurlReq::setup();
	    OrderHandling::DeleteOrder(args[3]);
	    CurlReq::q.WaitUntilQueueEmpty();
	    CurlReq::disconnect();
	}
	else {
	    std::cout << "invalid command. \nNOTE:all web operation require your jwt token in the last argument" << std::endl;
	}
    }
    else {
	if (args.size() < 3) {
	    std::cout << "missing jwt token" << std::endl;
	    return;
	}
    	std::thread InGameTrades(OrderHandling::EElogChecking);
	WebLoop(args[2]);
	InGameTrades.join();
    }
}

void CommandDispatch(std::vector<std::string> args){
    if(args.size() == 1){
	std::cout << "no command given" << std::endl;
	return;
    }
    if((std::string)args[1] == "run"){
	Run(args);
    }
    else if ((std::string)args[1] == "set") {
    
    }
    else {
	std::cout << "command not recognized" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    CommandDispatch(ParseArgv(argc, argv));
    /*
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
    }*/
    return 0;
}
