#include "CurlReq.hpp"
#include "Sorting.hpp"
#include <fstream>
// std::chrono::steady_clock::time_point last = std::chrono::steady_clock::now();
// while(true){
//     if(std::chrono::steady_clock::now() >= last + std::chrono::milliseconds(3000)) break;
// }
int main(int argc, char* argv[]) {
    CURL_OP::setup();
    std::cout << "runs" << std::endl;
    // json res = CURL_OP::GETjson("https://api.warframe.market/v2/orders/my", {"Content-Type: application/json", "Accept: application/json", "Authorization: Bearer JWT"}); 
    // POSTjson("https://api.warframe.market/v2/order", argv[1]);    
    json items = CURL_OP::GETjson("https://api.warframe.market/v2/items", {"accept: application/json", "Language: en"});
    while (true)
    {
        int counter = 0;
        for(json item: items["data"]){
            std::vector<std::string> tags = item["tags"];
            // std::cout << item["slug"] << std::endl;
            if(Sorting::ValidTrade(item["slug"], tags)){
                std::cout << item["slug"] << std::endl;
            }
        }
    }
    CURL_OP::disconnect();
    return 0;
}
