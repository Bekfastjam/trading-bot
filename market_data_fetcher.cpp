#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string readApiKey(const std::string& filename) {
    std::ifstream file(filename);
    std::string key;
    if (file.is_open()) {
        std::getline(file, key);
        file.close();
    } else {
        std::cerr << "Failed to open API key file: " << filename << std::endl;
        exit(1);
    }
    return key;
}

std::string fetchMarketData(const std::string& symbol, const std::string& apiKey) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    std::string url = "https://www.alphavantage.co/query?function=TIME_SERIES_INTRADAY"
                      "&symbol=" + symbol +
                      "&interval=5min&apikey=" + apiKey;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

int main() {
    std::string apiKey = readApiKey("alpha.key");
    std::string symbol = "AAPL";  // You can change this to any stock ticker

    std::cout << "Using API Key: " << apiKey << std::endl;

    std::string response = fetchMarketData(symbol, apiKey);
    std::cout << "Raw API response:\n" << response << std::endl;

    try {
        auto jsonData = json::parse(response);

        if (jsonData.contains("Time Series (5min)")) {
            auto timeSeries = jsonData["Time Series (5min)"];
            auto latest = timeSeries.begin();
            std::string timestamp = latest.key();
            std::string price = latest.value()["1. open"];
            std::cout << "Latest price for " << symbol << " at " << timestamp << ": $" << price << std::endl;
        } else {
            std::cerr << "Expected 'Time Series (5min)' not found in response.\n";
            std::cerr << "Full response:\n" << jsonData.dump(2) << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }

    return 0;
}
