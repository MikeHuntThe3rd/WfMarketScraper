#include "Sorting.hpp"
namespace Sorting {
    std::string slug = "";
    bool Frequency(itemType type, std::optional<std::any> data){
        json stats = CURL_OP::GETjson("https://api.warframe.market/v1/items/" + (std::string)slug  + "/statistics", {"accept: application/json", "Language: en"});
        int vol = 0;
        switch (type)
        {
        case itemType::basic :
            for(json curr : stats["payload"]["statistics_closed"]["48hours"]){
                vol += (int)curr["volume"];
            }
            break;
        case itemType::mod :
        {
            int rank = std::any_cast<int>(data.value());
            for(json curr : stats["payload"]["statistics_closed"]["48hours"]){
                if((int)curr["mod_rank"] == rank){
                    vol += (int)curr["volume"];
                }
            }
            break;
        }
        case itemType::Ayatan :
        {
            int cyan = std::any_cast<ayatan_sculpture>(data.value()).cyanStars;
            int amber = std::any_cast<ayatan_sculpture>(data.value()).amberStars;
            for(json curr : stats["payload"]["statistics_closed"]["48hours"]){
                    if((int)curr["cyan_stars"] == cyan && (int)curr["cyan_stars"] == amber){
                        vol += (int)curr["volume"];
                    }
                }
            break;
        }
        default:
            break;
        }
        
        if(vol > 96){
            // std::cout << "volume" << vol << std::endl;
            return true;
        }
        else{
            // std::cout << "volume" << vol << std::endl;
            return false;
        }
    }
    std::optional<int> RankBasedMargin(json orders){
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
        if(margin > 10 && Frequency(itemType::mod , best_margin_key)){
            //std::cout << "rank: " << best_margin_key << std::endl;
            // std::cout << "sell:" << ranks[best_margin_key].sell << std::endl;
            // std::cout << "buy:" << ranks[best_margin_key].buy << std::endl;
            return best_margin_key;
        }
        else{
            return std::nullopt;
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
        if((sell_trade && buy_trade) && sell - buy > 10 && Frequency(itemType::basic)){
            // std::cout << "sell:" << sell << std::endl;
            // std::cout << "buy:" << buy << std::endl;
            return true;
        }
        else{
            return false;
        }
    }
    std::optional<ayatan_sculpture> AyatanMargin(json orders){
        bool buy_trade = false, sell_trade = false;
        std::vector<ayatan_sculpture> sculptures;
        auto findElement = [&sculptures](int cyanStars, int amberStars){
            auto match = std::find_if(sculptures.begin(), sculptures.end(), 
            [&cyanStars, &amberStars](const ayatan_sculpture& sculpture)
            {
                return sculpture.cyanStars == cyanStars && sculpture.amberStars == amberStars;
            });
            return match;
        };
        auto addIfNew = [&sculptures, findElement](int cyanStars, int amberStars){
            if(findElement(cyanStars, amberStars) == sculptures.end()){
                sculptures.push_back({cyanStars, amberStars});
            }
        };
        auto priceCompare = [&sculptures, findElement](int cyanStars, int amberStars, tradeType type, int value){
            auto element = findElement(cyanStars, amberStars);
            if(element != sculptures.end()){
                switch (type)
                {
                case tradeType::sell :
                    if(element->sell > value){
                        element->sell = value;
                        element->sell_trade = true;
                    }
                    break;
                case tradeType::buy :
                    if(element->buy < value){
                        element->buy = value;
                        element->buy_trade = true;
                    }
                    break;
                default:
                    break;
                }
            }
        };
        for(json order : orders["data"]){
            int cyan = order["cyanStars"];
            int amber = order["amberStars"];
            addIfNew(cyan, amber);

            if(order["user"]["status"] == "ingame" && order["type"] == "sell"){
                priceCompare(cyan, amber, tradeType::sell, order["platinum"]);
            }
            if(order["user"]["status"] == "ingame" && order["type"] == "buy"){
                priceCompare(cyan, amber, tradeType::sell, order["platinum"]);
            }
        }
        int margin = std::numeric_limits<int>::min();
        std::optional<ayatan_sculpture> best = std::nullopt;
        for(ayatan_sculpture SC : sculptures){
            if((SC.sell_trade && SC.buy_trade) && SC.sell - SC.buy > margin){
                margin = SC.sell - SC.buy;
                best = SC;
            }
        }
        if(best.has_value() && margin > 10 && Frequency(itemType::Ayatan, best)) return best;
        else return std::nullopt;
    }
    void ValidTrade(std::string item, std::vector<std::string> tags){
        slug = item;
        json orders = CURL_OP::GETjson("https://api.warframe.market/v2/orders/item/" + (std::string)slug, {"accept: application/json", "Language: en"});
        if(std::find(tags.begin(), tags.end(), "ayatan_sculpture") != tags.end()){
            std::optional<ayatan_sculpture> result = AyatanMargin(orders);
            if(result.has_value()) std::cout << result->buy << "AYATAN!!!!!!";
            else std::cout << "not today amigo";
        }
        else if(std::find(tags.begin(), tags.end(), "rank") != tags.end()){
            std::optional<int> result = RankBasedMargin(orders);
            if(result.has_value()) std::cout << *result << "MOD!!!!!!";
            else std::cout << "not today amigo";
        }
        else{
            if(BasicMargin(orders)) std::cout << "BASIC!!!!!!";
            else std::cout << "not today amigo";
        }
        
    }
}
