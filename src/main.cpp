#include "CurlReq.hpp"
#include "Sorting.hpp"
#include <fstream>
// std::chrono::steady_clock::time_point last = std::chrono::steady_clock::now();
// while(true){
//     if(std::chrono::steady_clock::now() >= last + std::chrono::milliseconds(3000)) break;
// }
int main(int argc, char* argv[]) {
    setup();
    std::cout << "runs" << std::endl;
    // json res = GETjson("https://api.warframe.market/v2/orders/my", {"Content-Type: application/json", "Accept: application/json", "Authorization: Bearer JWT"}); 
    POSTjson("", argv[1]);    
    // json items = GETjson("https://api.warframe.market/v2/items", {"accept: application/json", "Language: en"});
    // while (true)
    // {
    //     int counter = 0;
    //     for(json item: items["data"]){
    //         // std::cout << item["slug"] << std::endl;
    //         if(CheckMargin(item["slug"])){
    //             std::cout << item["slug"] << std::endl;
    //         }
    //     }
    // }
    disconnect();
    return 0;
}
