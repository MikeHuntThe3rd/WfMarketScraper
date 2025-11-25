#ifdef _WIN32
    #define NOMINMAX
#endif
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <curl/curl.h>
#include "json.hpp"

using json = nlohmann::json;

//variables
extern CURL* curl;
extern std::string response_string;
extern std::chrono::milliseconds interval;
// struct POSTbodyDATA {

// };
//functions
void setup();
void disconnect();
void SETcurlData(std::string url, std::vector<std::string> headers);
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);
json GETjson(std::string https, std::vector<std::string> headers = {});
void POSTjson(std::string https, std::string JWT);