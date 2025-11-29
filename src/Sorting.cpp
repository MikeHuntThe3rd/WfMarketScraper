#include "Sorting.hpp"
namespace Sorting {
    std::string slug = "";
    bool Frenquency(std::optional<int> rank){
        json stats = CURL_OP::GETjson("https://api.warframe.market/v1/items/" + (std::string)slug  + "/statistics", {"accept: application/json", "Language: en"});
        int vol = 0;
        for(json curr : stats["payload"]["statistics_closed"]["48hours"]){
            if(rank.has_value() && rank.value() == (int)curr["mod_rank"]){
                vol += (int)curr["volume"];
            }
            else{
                vol += (int)curr["volume"];
            }
        }
        if(vol > 96){
            std::cout << "volume" << vol << std::endl;
            if(rank.has_value()) std::cout << "rank" << rank.value();
            return true;
        }
        else{
            std::cout << "volume" << vol << std::endl;
            return false;
        }
    }
    bool RankBasedMargin(json orders){
        struct rank
        {
            bool sell_trade = false, buy_trade = false;
            int sell = std::numeric_limits<int>::max(), buy = std::numeric_limits<int>::min();
        };
        std::unordered_map<int, rank> ranks;
        for(json curr: orders["data"]){
            int key = curr["rank"];
            if(ranks.find(key) == ranks.end()){
                ranks.insert({key, rank()});
            }
            if(curr["platinum"] < ranks[key].sell && curr["user"]["status"] == "ingame" && curr["type"] == "sell"){
                ranks[key].sell_trade = true;
                ranks[key].sell = curr["platinum"];
            }
            if(curr["platinum"] > ranks[key].buy && curr["user"]["status"] == "ingame" && curr["type"] == "buy"){
                ranks[key].buy_trade = true;
                ranks[key].buy = curr["platinum"];
            }
        }
        int margin = std::numeric_limits<int>::min();
        int best_margin_key;
        for(const auto& rank: ranks){
            if((rank.second.buy_trade && rank.second.sell_trade) && margin < rank.second.sell - rank.second.buy){
                margin = rank.second.sell - rank.second.buy;
                best_margin_key = rank.first;
            }
        }
        if(margin > 10 && Frenquency({best_margin_key})){
            //std::cout << "rank: " << best_margin_key << std::endl;
            // std::cout << "sell:" << ranks[best_margin_key].sell << std::endl;
            // std::cout << "buy:" << ranks[best_margin_key].buy << std::endl;
            return true;
        }
        else{
            return false;
        }
    }
    bool BasicMargin(json orders){
        int buy = std::numeric_limits<int>::min();
            int sell = std::numeric_limits<int>::max();
            bool buy_trade = false, sell_trade = false;
            for(json curr: orders["data"]){
                if(curr["platinum"] < sell && curr["user"]["status"] == "ingame" && curr["type"] == "sell"){
                    sell = curr["platinum"];
                    sell_trade = true;
                }
                if(curr["platinum"] > buy && curr["user"]["status"] == "ingame" && curr["type"] == "buy"){
                    buy = curr["platinum"];
                    buy_trade = true;
                }
            }
            if(sell - buy > 10 && (sell_trade && buy_trade) && Frenquency()){
                // std::cout << "sell:" << sell << std::endl;
                // std::cout << "buy:" << buy << std::endl;
                return true;
            }
            else{
                return false;
            }
    }
    bool AyatanMargin(json orders){
        return true;
    }
    bool ValidTrade(std::string item, std::vector<std::string> tags){
        slug = item;
        json orders = CURL_OP::GETjson("https://api.warframe.market/v2/orders/item/" + (std::string)slug, {"accept: application/json", "Language: en"});
        if(std::find(tags.begin(), tags.end(), "ayatan_sculpture") != tags.end()){
            return AyatanMargin(orders);
        }
        else if(std::find(tags.begin(), tags.end(), "rank") != tags.end()){
            return RankBasedMargin(orders);
        }
        else{
            return BasicMargin(orders);
        }
        
    }
}