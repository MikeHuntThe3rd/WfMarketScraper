#pragma once
#include <iostream>
#include <ios>
#include <vector>
#include <string>
#include <chrono>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include "json.hpp"
#include "Types.hpp"
#include <curl/curl.h>


using json = nlohmann::json;
namespace CurlReq {
    //variables
    inline CURL* curl = nullptr;
    inline std::string response_string;
    inline std::chrono::steady_clock::time_point start{std::chrono::steady_clock::now()};
    inline std::chrono::milliseconds interval{334};
    //functions
    void setup();
    void disconnect();
    void wait();
    void SETcurlData(std::string url, std::vector<std::string> headers);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);
    //methods
    json __GET(std::string https, std::vector<std::string> headers = {"accept: application/json", "Language: en"});
    json __POST(std::string https, std::string JWT, std::string body);
    json __DELETE(std::string https, std::string JWT);
    json __PATCH(std::string https, std::string JWT, std::string body);
    //queuing class
    class Queuing{
        public:
            Queuing();
            std::future<json> Add(std::function<json()> f);
        private: 
            std::queue<Types::Task> tasks;
            std::mutex thread_lock;
            std::condition_variable cv;
            std::thread worker;
            
            void QueueUnloading();
    };
    inline Queuing q{};
}
