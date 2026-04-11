#pragma once
#include "VARS.hpp"
#include "json.hpp"
#include <chrono>
#include <condition_variable>
#include <curl/curl.h>
#include <fstream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

using json = nlohmann::json;
namespace CurlReq {
// variables
inline CURL *curl = nullptr;
inline std::string response_string;
inline std::chrono::steady_clock::time_point start{
    std::chrono::steady_clock::now()};
inline std::chrono::milliseconds interval{334};
// functions
void setup();
void disconnect();
void wait();
void SETcurlData(std::string url, std::vector<std::string> headers);
static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            std::string *data);
// methods
json __GET(std::string https, std::vector<std::string> headers = {
                                  "accept: application/json", "Language: en"});
json __POST(std::string https, std::string body);
json __DELETE(std::string https);
json __PATCH(std::string https, std::string body);
// queuing class
class Queuing {
public:
  Queuing();
  ~Queuing();
  std::future<json> Add(std::function<json()> f);
  void WaitUntilQueueEmpty();

private:
  std::queue<VARS::Task> tasks;
  std::mutex thread_lock;
  std::condition_variable cv;
  std::thread worker;

  void QueueUnloading();
};
inline Queuing q{};
} // namespace CurlReq
