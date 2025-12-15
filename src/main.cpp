#include "CurlReq.hpp"
#include "Sorting.hpp"
#include "OrderHandling.hpp"
#include <fstream>

int main(int argc, char* argv[]) {
    CurlReq::setup();
    std::cout << "runs" << std::endl;

    std::ofstream del("out.log", std::ios::trunc);
    del.close();
    OrderHandling::DeleteOrder(argv[1]);
    json items = CurlReq::GETjson("https://api.warframe.market/v2/items", {"accept: application/json", "Language: en"});
    for(json item: items["data"]){
        auto trade = Sorting::ValidTrade(item["slug"], item["tags"].get<std::vector<std::string>>(), true);
        if(trade.good_trade){
            std::string id = item["id"].get<std::string>();
            OrderHandling::PostOrder(argv[1], id, trade);
        }
    }
    CurlReq::disconnect();
    return 0;
}
