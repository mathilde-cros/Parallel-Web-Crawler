#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <regex>
#include <curl/curl.h>
#include <thread>

// // SetList data structure to store URLs
// class SetList {
// private:
//     std::vector<std::string> urls;
// public:
//     // Add a URL to the list
//     void addURL(const std::string& url) {
//         urls.push_back(url);
//     }
//     // Display all URLs in the list
//     void displayURLs() {
//         for(std::vector<std::string>::const_iterator it = urls.begin(); it != urls.end(); ++it) {
//             const std::string& url = *it;
//             std::cout << url << std::endl;
//         }
//     }
//     // Check if a URL is present in the list
//     bool containsURL(const std::string& url) {
//         return std::find(urls.begin(), urls.end(), url) != urls.end();
//     }
//     // Clear the list of URLs
//     void clearList() {
//         urls.clear();
//     }
//     // Remove a specific URL from the list
//     void removeURL(const std::string& url) {
//         std::vector<std::string>::const_iterator it = std::find(urls.begin(), urls.end(), url);
//         if(it != urls.end()) {
//             urls.erase(it);
//             std::cout << "URL removed successfully." << std::endl;
//         } else {
//             std::cout << "URL not found." << std::endl;
//         }
//     }
// };

// Callback function to receive HTTP response
size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

// Function to fetch HTML content from a URL
std::string fetchHTML(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string data;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
        return data;
    } else {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return "";
    }
}

// Function to extract URLs from crawling the HTML content using regex
std::set<std::string> crawl(const std::string& html) {
    std::set<std::string> urls;
    // Regular expression to find URLs in HTML
    std::regex urlRegex(R"(href=["'](.*?)["'])");
    auto words_begin = std::sregex_iterator(html.begin(), html.end(), urlRegex);
    auto words_end = std::sregex_iterator();
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string url = match[1].str();
        urls.insert(url);
    }
    return urls;
}

int main() {
    std::string url;
    std::cout << "Enter the URL: ";
    std::cin >> url;

    std::string html = fetchHTML(url);
    std::set<std::string> urlSet = crawl(html);

    std::cout << "URLs found in the page:" << std::endl;
    for (const auto& u : urlSet) {
        std::cout << u << std::endl;
    }

    return urlSet.size();
}