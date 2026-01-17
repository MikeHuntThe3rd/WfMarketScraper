#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <thread>
#include <vector>
#include "json.hpp"
#include "CurlReq.hpp"
#include "Sorting.hpp"
#include "OrderHandling.hpp"
#include "Types.hpp"
#include "optionparser.h"

struct MapElement {
    std::string key;
    int value;
};

std::vector<std::string> ParseArgv(int argc, char* argv[]){
    std::vector<std::string> result(argv, argv + argc);
    return result;
}

void ChangeSettings(std::optional<MapElement> change = std::nullopt){
    //set change if given
    if(change.has_value()) Types::Settings[change->key] = change->value;
    //remake the entire json with the settings unordered map as a template
    using json = nlohmann::json;
    json def;
    for(auto element : Types::Settings){
	def[element.first] = element.second;
    }
    std::ofstream DefSettings("settings.json");
    DefSettings << def;
    DefSettings.close();
}

void CreateSettings(){
    std::string fileName = "settings.json";
    std::ifstream file(fileName.c_str());
    if(!file.good()){
	std::cout << "settings generated with default settings" << std::endl;
	ChangeSettings();
    }
}

void SettingsSetup(){
    using json = nlohmann::json;
    CreateSettings();
    std::ifstream in("settings.json");
    json data = json::parse(in);
    for(const auto & [key, value] : data.items()){
	auto setting = Types::Settings.find(key);
	if(setting != Types::Settings.end()){
	    try {
		setting->second = value;
	    } 
	    catch (std::exception e) {
		std::cout << e.what() << std::endl;
	    }
	}
    }
}

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


void Set(std::vector<std::string> args){
    if(args.size() > 4){
	std::cout << "too many arguments given" << std::endl;
	return;
    }
    std::optional<std::string> mode = (args.size() >= 3) ? std::optional<std::string>{(std::string)args[2]} : std::nullopt;
    if(mode.has_value()){
	CreateSettings();
	auto setting = Types::Settings.find(mode.value());
	int value;
	try {
	    if(args.size() < 4){
		std::cout << "no value given" << std::endl;
		return;
	    }
	    value = std::stoi(args[3]);
	} catch (std::exception e) {
	    std::cout << "error: " << e.what() << std::endl;
	    return;
	}
	if(setting != Types::Settings.end()){
	    SettingsSetup();
	    ChangeSettings(MapElement{setting->first, value});
	    std::cout << "setting changed succesfully" << std::endl;
	}
	else {
	    std::cout << "set mode not found" << std::endl;
	    return;
	}

    }
    else {
	std::cout << "no set mode given" << std::endl;
    }
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
	    SettingsSetup();
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
	Set(args);
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
