#include "Sorting.hpp"
namespace Sorting {
    std::string slug = "";
    std::ofstream logfile;
    bool Frequency(itemType type, std::optional<std::any> data){
        json stats = CURL_OP::GETjson("https://api.warframe.market/v1/items/" + (std::string)slug  + "/statistics", {"accept: application/json", "Language: en"});
        int vol = 0;
        switch (type)
        {
        case itemType::basic :
            logfile << "----------checking frequency for basic item" << std::endl;
            for(json curr : stats["payload"]["statistics_closed"]["48hours"]){
                vol += (int)curr["volume"];
            }
            logfile << "volume min(96): " << vol << std::endl;
            break;
        case itemType::mod :
        {
            int rank = std::any_cast<int>(data.value());
            logfile << "----------checking frequency for mod item" << std::endl;
            for(json curr : stats["payload"]["statistics_closed"]["48hours"]){
                if((int)curr["mod_rank"] == rank){
                    vol += (int)curr["volume"];
                }
            }
            logfile << "volume min(96): " << vol << std::endl;
            break;
        }
        case itemType::Ayatan :
        {
            int cyan = std::any_cast<ayatan_sculpture>(data.value()).cyanStars;
            int amber = std::any_cast<ayatan_sculpture>(data.value()).amberStars;
            logfile << "----------checking frequency for ayatan sculpture" << std::endl;
            for(json curr : stats["payload"]["statistics_closed"]["48hours"]){
                if((int)curr["cyan_stars"] == cyan && (int)curr["cyan_stars"] == amber){
                    vol += (int)curr["volume"];
                }
            }
            logfile << "volume min(96): " << vol << std::endl;
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
            int key;
            try
            {
                key =  curr["rank"];
            }
            catch(const nlohmann::json::exception& e)
            {
                std::cout << e.what() << '\n';
                return std::nullopt;
            }
            
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
        bool value_found = false;
        for(const auto& rank: ranks){
            if((rank.second.buy_trade && rank.second.sell_trade) && margin < rank.second.sell - rank.second.buy){
                margin = rank.second.sell - rank.second.buy;
                best_margin_key = rank.first;
                value_found = true;
            }
        }
        if(value_found){
            logfile << "margin: " << margin << std::endl;
            logfile << "rank: " << best_margin_key << std::endl;
        }
        else logfile << "no good mod trade found: " << std::endl;
        
        if(value_found && margin > 10 && Frequency(itemType::mod , best_margin_key)){
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
        logfile << "found sell, buy: "<< sell_trade << buy_trade << std::endl;
        logfile << "margin: " << sell - buy << std::endl;
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
                logfile << "Added new vector element (stars: c, a): " << cyanStars + " " + amberStars << std::endl;
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
                        logfile << "Updated a vector element type sell (stars: c, a): " << cyanStars + " " + amberStars << std::endl;
                    }
                    break;
                case tradeType::buy :
                    if(element->buy < value){
                        element->buy = value;
                        element->buy_trade = true;
                        logfile << "Updated a vector element type buy (stars: c, a): " << cyanStars + " " + amberStars << std::endl;
                    }
                    break;
                default:
                    break;
                }
            }
        };
       
        for(json order : orders["data"]){
            int cyan = 0;
            int amber = 0;
            try
            {
                cyan = order["cyanStars"];
            }
            catch(...)
            {
            }
            try
            {
                amber = order["amberStars"];
            }
            catch(...){}
            
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
        logfile << "resulting margin: " << margin << std::endl;
        if(best.has_value()) logfile << "struct of the best sculpture (stars: c, a):" << best->cyanStars + " " + best->amberStars << std::endl;
        else logfile << "best is empty" << std::endl;

        if(best.has_value() && margin > 10 && Frequency(itemType::Ayatan, best)){
            return best;
        } 
        else {
            return std::nullopt;
        } 
    }
    void ValidTrade(std::string item, std::vector<std::string> tags, bool log){
        slug = item;
        if(log){
            // logfile.open("out.log", std::ios::trunc);
            // logfile.close();
            logfile.open("out.log", std::ios::app);
        }
        json orders = CURL_OP::GETjson("https://api.warframe.market/v2/orders/item/" + (std::string)slug, {"accept: application/json", "Language: en"});
        if(std::find(tags.begin(), tags.end(), "ayatan_sculpture") != tags.end()){
            logfile << "==========AYATAN CHECK==========" << std::endl;
            logfile << "slug: " << slug << std::endl;

            std::optional<ayatan_sculpture> result = AyatanMargin(orders);
            if(result.has_value()){
                logfile << "AYATAN!!!!!!" << std::endl;
                std::cout << result->buy << "AYATAN!!!!!!" << std::endl;
            }
        }
        else if(std::find(tags.begin(), tags.end(), "mod") != tags.end() && std::find(tags.begin(), tags.end(), "veiled_riven") == tags.end()){
            logfile << "==========MOD CHECK==========" << std::endl;
            logfile << "slug: " << slug << std::endl;

            std::optional<int> result = RankBasedMargin(orders);
            if(result.has_value()){
                logfile << "MOD!!!!!!" << std::endl;
                std::cout << *result << "MOD!!!!!!" << std::endl;
            }
        }
        else{
            logfile << "==========BASIC CHECK==========" << std::endl;
            logfile << "slug: " << slug << std::endl;
            if(BasicMargin(orders)){
                logfile << "BASIC!!!!!!" << std::endl;
                std::cout << "BASIC!!!!!!" << std::endl;
            } 
        }
        logfile.close();
    }
}
