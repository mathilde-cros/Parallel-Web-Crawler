#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <curl/curl.h>
#include "hashtable.cpp"

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
void crawl(std::string &url, SetList &urlSet) {
    std::string html = fetchHTML(url);
    if (html.empty()) {
        return;
    }
    // Regular expression to find URLs in HTML
    std::regex urlRegex(R"(href=["']([^"']+)["'])");
    auto words_begin = std::sregex_iterator(html.begin(), html.end(), urlRegex);
    auto words_end = std::sregex_iterator();
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string url2 = match[1].str();
        if (urlSet.containsURL(url2)){
            continue;
        }
        urlSet.addURL(url2);
        crawl(url2, urlSet);
    }
}

int main(int argc, char* argv[]) {

    if (argc != 2){
        std::cerr << "Invalid command, please give as argument the url you want to crawl" << std::endl;
        return 1;
    }

    std::string url = argv[1];
    // Add as argument hash_table (when created)
    // Add as argument to keep only url starting like first one!

    // Validate the URL format
    std::regex urlFormat(R"(https?://\S+)");
    if (!std::regex_match(url, urlFormat)) {
        std::cerr << "Invalid URL format. Please enter a valid URL starting with http:// or https://" << std::endl;
        return 1;
    }

    SetList urlSet;
    urlSet.addURL(url);
    crawl(url, urlSet);
    std::cout << "URLs found" << std::endl;
    urlSet.displayList();

    return 0;
}