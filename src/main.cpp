#include "CurlReq.hpp"
#include "Sorting.hpp"
#include "OrderHandling.hpp"
#include <fstream>
// std::chrono::steady_clock::time_point last = std::chrono::steady_clock::now();
// while(true){
//     if(std::chrono::steady_clock::now() >= last + std::chrono::milliseconds(3000)) break;
// }
int main(int argc, char* argv[]) {
    CurlReq::setup();
    std::cout << "runs" << std::endl;
    std::ofstream del("out.log", std::ios::trunc);
    del.close();
    // CURL_OP::POSTjson("https://api.warframe.market/v2/order", argv[1]);    
    json items = CurlReq::GETjson("https://api.warframe.market/v2/items", {"accept: application/json", "Language: en"});
    for(json item: items["data"]){
        auto trade = Sorting::ValidTrade(item["slug"], item["tags"].get<std::vector<std::string>>(), true);
        if(trade.good_trade){
            std::string id = item["id"].get<std::string>();
            OrderHandling::PostOrder(id, argv[1], trade);
        }
    }
    CurlReq::disconnect();
    return 0;
}
