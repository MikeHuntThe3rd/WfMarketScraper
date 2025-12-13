#include "CurlReq.hpp"

namespace CURL_OP {
	CURL* curl = nullptr;
	std::string response_string;
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	std::chrono::milliseconds interval = std::chrono::milliseconds(334);
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
	json GETjson(std::string https, std::vector<std::string> headers){
		wait();
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		SETcurlData(https, headers);
		CURLcode response = curl_easy_perform(curl);
		if (response != CURLE_OK) {
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(response) << std::endl;
			response_string.clear();
			return nullptr;
		}
		else if(response_string.length() == 0){
			std::cout << "no result" << std::endl;
			return nullptr;
		}
		else {
			//std::cout << interval.count() << endl;
			try {
				json data = json::parse(response_string);
				// js << data.dump(4);
				response_string.clear();
				return data;
			} catch (nlohmann::json::parse_error& err) {
				//std::cout << response_string;
				std::string error;
				std::cin >> error;
			}
		}
	}
	void POSTjson(std::string https, std::string JWT, std::string body = ""){
		//body is a very specific kind of string: R"({}) make sure you format everything like this for it to work
		wait();
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		SETcurlData(https, {"Content-Type: application/json", "Accept: application/json", "Authorization: Bearer " + JWT});
		body = R"({
			"itemId": "59eba86952f90ed715e1415a",
			"type": "sell",
			"platinum": 38,
			"quantity": 1,
			"visible": false,
			"perTrade": 1
		})";
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
		} else {
			std::ofstream out("return.json");
			json temp = response_string;
			out << temp.dump(4);
			out.close();
		}
	}
	void SETcurlData(std::string url, std::vector<std::string> headers){
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		struct curl_slist* headr = NULL;
		for(std::string header: headers){
			headr = curl_slist_append(headr, header.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headr);
	}
}
