#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <fstream>
#include "json.hpp"
#include <curl/curl.h>

using json = nlohmann::json;
namespace CurlReq {
    //variables
    extern CURL* curl;
    extern std::string response_string;
    extern std::chrono::steady_clock::time_point start;
    extern std::chrono::milliseconds interval;
    //functions
    void setup();
    void disconnect();
    void wait();
    void SETcurlData(std::string url, std::vector<std::string> headers);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);
    //methods
    json GETjson(std::string https, std::vector<std::string> headers = {});
    void POSTjson(std::string https, std::string JWT, std::string body);
    void __DELETE(std::string https, std::string JWT);
    void __PATCH(std::string https, std::string JWT, std::string body);
}
