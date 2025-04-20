#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

std::string readApiKey(const std::string& filename) {
    std::ifstream file(filename);
    std::string key;
    if (file && std::getline(file, key)) {
        return key;
    } else {
        std::cerr << "Failed to read API key from " << filename << std::endl;
        exit(1);
    }
}

json fetchStockPrice(const std::string& symbol, const std::string& apiKey) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        std::string url = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=" 
                          + symbol + "&apikey=" + apiKey;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    return json::parse(response);
}

int main() {
    std::string symbol = "AAPL";
    std::string apiKey = readApiKey("alpha.key");

    json data = fetchStockPrice(symbol, apiKey);

    if (data.contains("Global Quote")) {
        std::cout << "Price for " << symbol << ": $" << data["Global Quote"]["05. price"] << std::endl;
    } else {
        std::cerr << "Failed to get quote. Response: " << data.dump(2) << std::endl;
    }

    return 0;
}
