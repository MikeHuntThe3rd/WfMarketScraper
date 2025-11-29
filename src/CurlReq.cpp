#include "CurlReq.hpp"

namespace CURL_OP {
	CURL* curl = nullptr;
	std::string response_string;
	std::chrono::milliseconds interval = std::chrono::milliseconds(100);
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
	json GETjson(std::string https, std::vector<std::string> headers){
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		std::chrono::steady_clock::time_point timer = std::chrono::steady_clock::now();
		while(true){
			if(std::chrono::steady_clock::now() >= timer + interval){
				timer = std::chrono::steady_clock::now();
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
		}
	}
	void POSTjson(std::string https, std::string JWT){
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		std::string body = R"({
			"itemId": "54aae292e7798909064f1575",
			"type": "sell",
			"platinum": 38,
			"quantity": 12,
			"visible": false
		})";
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
		SETcurlData(https, {"Content-Type: application/json", "Accept: application/json", "Authorization: Bearer " + JWT});
		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
		} else {
			// std::cout << "Response:\n" << response_string << "\n";
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
