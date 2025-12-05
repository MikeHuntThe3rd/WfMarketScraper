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
    // CURL_OP::POSTjson("https://api.warframe.market/v2/order", argv[1]);    
    json items = CURL_OP::GETjson("https://api.warframe.market/v2/items", {"accept: application/json", "Language: en"});
    while (true)
    {
        for(json item: items["data"]){
            std::vector<std::string> tags = item["tags"];
            // std::cout << item["slug"] << std::endl;
            Sorting::ValidTrade(item["slug"], tags);
        }
    }
    CURL_OP::disconnect();
    return 0;
}
