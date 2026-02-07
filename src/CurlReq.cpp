#include "CurlReq.hpp"
#include <iostream>
#include <mutex>

namespace CurlReq {
	void setup(){
		curl_global_init(CURL_GLOBAL_DEFAULT);
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	}
	void disconnect(){
		curl_easy_cleanup(curl); 
		curl_global_cleanup();
	}
	size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data){
		size_t total_size = size * nmemb;
		data->append((char*)contents, total_size);
		return total_size;
	}
	void wait(){
		auto now = std::chrono::steady_clock::now();
		auto phase = std::chrono::duration_cast<std::chrono::milliseconds>(now - start) % interval;
		auto actual_pause_time = interval - phase;
		std::this_thread::sleep_until(now + actual_pause_time);
	}
	void SETcurlData(std::string url, std::vector<std::string> headers){
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		struct curl_slist* headr = NULL;
		for(std::string header: headers){
			headr = curl_slist_append(headr, header.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headr);
	}
	//methods
	json __GET(std::string https, std::vector<std::string> headers){
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		SETcurlData(https, headers);
		CURLcode response = curl_easy_perform(curl);
		if (response != CURLE_OK) {
			std::cerr << "GET failed: " << curl_easy_strerror(response) << std::endl;
			response_string.clear();
			return json{};
		}
		else if(response_string.length() == 0){
			std::cout << "no result" << std::endl;
			return json{};
		}
		else {
			//std::cout << interval.count() << endl;
			try {
				json data = json::parse(response_string);
				// js << data.dump(4);
				response_string.clear();
				return data;
			} catch (nlohmann::json::parse_error& err) {
			    std::cout << err.what();
			}
		}
	}
	json __POST(std::string https, std::string body){
		//body is a very specific kind of string: R"({}) make sure you format everything like this for it to work
		// std::string bdy = R"({
		// 	"itemId": "56783f24cbfa8f0432dd899c",
		// 	"platinum": 80,
		// 	"quantity": 1,
		// 	"type": "buy",
		// 	"visible": false
        // })";
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		SETcurlData(https, {"Content-Type: application/json", "Accept: application/json", "Authorization: Bearer " + VARS::JWT});
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
		CURLcode res = curl_easy_perform(curl);
		std::ofstream logfile("out.log", std::ios::app);
		if (res != CURLE_OK) {
			std::cerr << "POST failed: " << curl_easy_strerror(res) << "\n";
			return json{};
		}
		else if(response_string.length() == 0){
			return json{};
		}
		else {
			json result = json::parse(response_string);

			// std::ofstream out("return.json");
			// json temp = response_string;
			// out << temp.dump(4);
			// out.close();

			// std::ofstream scnd("body.json");
			// temp = body;
			// scnd << temp.dump(4);
			// scnd.close();
			response_string.clear();
			return result;
		}
	}
	json __DELETE(std::string https){
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
		SETcurlData(https, {"Content-Type: application/json", "Accept: application/json", "Authorization: Bearer " + VARS::JWT});
		CURLcode response = curl_easy_perform(curl);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, nullptr);
		if(response != CURLE_OK){
			std::cerr << "DELETE failed: " << curl_easy_strerror(response) << "\n";
			return json{};
		}
		else if(response_string.length() == 0){
			return json{};
		}
		else{
			std::ofstream out("delete_response.json");
			// json j = response_string;
			// out << j.dump(4);
			// out.close();
			json result = json::parse(response_string);
			response_string.clear();
			return result;
		}
	}
	json __PATCH(std::string https, std::string body){
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
		SETcurlData(https, {"Content-Type: application/json", "Accept: application/json", "Authorization: Bearer " + VARS::JWT});
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
		CURLcode response = curl_easy_perform(curl);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, nullptr);
		if(response != CURLE_OK){
			std::cerr << "PATCH failed: " << curl_easy_strerror(response) << "\n";
			return json{};
		}
		else if(response_string.length() == 0){
			return json{};
		}
		else{
			json result = json::parse(response_string);
			// std::ofstream out("patch_response.json");
			// json j = response_string;
			// out << j.dump(4);
			// out.close();
			response_string.clear();
			return result;
		}
	}
	//queuing
	Queuing::Queuing(){
		worker = std::thread([this]{ QueueUnloading(); });
	}	
	void Queuing::QueueUnloading() {
		VARS::Task task;
		while(true){
			std::unique_lock<std::mutex> lock(thread_lock);
			cv.wait(lock, [&]{return !tasks.empty();});
			task = std::move(tasks.front());
			tasks.pop();
			cv.notify_all();
			json result = task.work();
			task.promise.set_value(result);
			CurlReq::wait();
		}
	}
	std::future<json> Queuing::Add(std::function<json()> f){
		VARS::Task task;
		task.work = std::move(f);
		std::future<json> future = task.promise.get_future();
		{
			std::lock_guard<std::mutex> lock(thread_lock);
			tasks.push(std::move(task));
		}
		cv.notify_one();
		return future;
	}
	void Queuing::WaitUntilQueueEmpty(){
	    std::unique_lock<std::mutex> lock(thread_lock);
	    cv.wait(lock, [&]{return tasks.empty();});
	}
}
