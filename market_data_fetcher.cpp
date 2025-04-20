#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// cURL write callback to collect API response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Read API key from a file
std::string read_api_key(const std::string& filename) {
    std::ifstream file(filename);
    std::string key;
    std::getline(file, key);
    return key;
}

int main() {
    std::string apiKey = read_api_key("alpha.key");
    std::string symbol = "AAPL"; // You can change this to any symbol
    std::string url = "https://www.alphavantage.co/query?function=TIME_SERIES_INTRADAY"
                      "&symbol=" + symbol +
                      "&interval=5min&apikey=" + apiKey;

    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
            return 1;
        }

        try {
            auto jsonData = json::parse(response);
            auto timeSeries = jsonData["Time Series (5min)"];

            // Get the latest timestamp (first entry)
            auto latest = timeSeries.begin();
            std::string timestamp = latest.key();
            std::string price = latest.value()["1. open"];

            std::cout << "Latest price for " << symbol << " at " << timestamp << ": $" << price << std::endl;
        } catch (std::exception& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            std::cerr << "Raw response:\n" << response << std::endl;
        }
    }

    return 0;
}
