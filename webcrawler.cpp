#include <iostream>
#include <string>
#include <set>
#include <regex>
#include <curl/curl.h>

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
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Failed to fetch URL: " << curl_easy_strerror(res) << std::endl;
            return ""; // Return empty string to indicate failure
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
    std::regex urlRegex(R"(href=["']([^"']+)["'])");
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

    // Example link to use: https://en.wikipedia.org/wiki/Concurrent_computing

    std::string url;
    std::cout << "Enter the URL: ";
    std::getline(std::cin, url); // Allowing for spaces in the URL

    // Validate the URL format
    std::regex urlFormat(R"(https?://\S+)");
    if (!std::regex_match(url, urlFormat)) {
        std::cerr << "Invalid URL format. Please enter a valid URL starting with http:// or https://" << std::endl;
        return 1;
    }

    std::string html = fetchHTML(url);
    if (html.empty()) {
        std::cerr << "No HTML content fetched. Exiting." << std::endl;
        return 1; // Error exit code
    }

    std::set<std::string> urlSet = crawl(html);
    std::cout << "URLs found in the page:" << std::endl;
    for (const auto& u : urlSet) {
        std::cout << u << std::endl;
    }
    return 0;
}