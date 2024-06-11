#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <curl/curl.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "hashtable.cpp"
#include "threadpool.cpp"

// Callback function to receive HTTP response
size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

// Function to check if URL exists
bool urlExists(const std::string& url) {
    CURL* curl;
    CURLcode res;
    long response_code = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // We don't need the body
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects

        res = curl_easy_perform(curl);
        
        if(res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if(response_code == 200) {
                curl_easy_cleanup(curl);
                curl_global_cleanup();
                return true;
            }
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return false;
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
            // std::cerr << "Failed to fetch URL: " << curl_easy_strerror(res) << std::endl;
            return ""; // Return empty string to indicate failure
        }
        curl_easy_cleanup(curl);
        return data;
    } else {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return "";
    }
}

// Parallel function to extract URLs from crawling the HTML content using regex
template <class T>
void crawl_parallel(std::string url, const std::string& base_url, T& urlSet, ThreadPool& threadPool, std::mutex& setMutex) {
    {
        std::lock_guard<std::mutex> lock(setMutex);
        if (!urlSet.addURL(url)) {
            return;
        }
    }

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

        // To ensure that the URL starts with the base URL
        if (url2.find(base_url) != 0) {
            if (url2.find('#') != std::string::npos 
                || url2.find("//") != std::string::npos 
                || url2.find(":") != std::string::npos 
                || url2.find("{") != std::string::npos){
                continue; // Pass if the url is an id on the page (#), another protocol (// or :) or a script ({)
            }else if (url2.find('/') == 0) { // Relative url 
                url2 = base_url + url2;
            } else if (url2.find('/') == std::string::npos){ // relative url
                url2 = base_url + '/' + url2;
            } else {
                continue;
            }
        }
        if (url2.find('#') != std::string::npos){ // Remove ids on page
            url2 = url2.substr(0, url2.find("#"));
        }
        if (url2.find('?') != std::string::npos){  // Remove arguments on page
            url2 = url2.substr(0, url2.find("?"));
        }

        {
            std::lock_guard<std::mutex> lock(setMutex);
            if (!urlSet.containsURL(url2)) {
                threadPool.add_task_to_queue([url2, base_url, &urlSet, &threadPool, &setMutex]() {
                    crawl_parallel(url2, base_url, urlSet, threadPool, setMutex);
                });
            }
        }
    }
}

int main(int argc, char* argv[]) {

    if (argc != 4){
        std::cerr << "Invalid command, please give your command as:" << std::endl;
        std::cerr << "./webcrawler <opt_set> <url> <num_threads>" << std::endl;
        std::cerr << "opt_set being 0 (SetList), 1 (CoarseHashTable), or 2 (StripedHashTable)" << std::endl;
        std::cerr << "\t\t defining the set you want to use to store the URLs" << std::endl;
        std::cerr << "url being the URL you want to crawl" << std::endl;
        std::cerr << "num_threads being the number of threads to use" << std::endl;
        return 1;
    }

    auto start = std::chrono::high_resolution_clock::now();

    int option_urlset = std::stoi(argv[1]);
    std::string url = argv[2];
    int num_threads = std::stoi(argv[3]);

    // Keep only url starting like first one in order to avoid crawling the whole internet (ex. redirects to instagram.com ...)!
    std::regex baseUrlRegex(R"((https?://[^/]+))");
    std::smatch baseUrlMatch;
    std::string base_url;
    if (std::regex_search(url, baseUrlMatch, baseUrlRegex)) {
        base_url = baseUrlMatch[1].str();
    } else {
        std::cerr << "Failed to extract base URL." << std::endl;
        return 1;
    }
    std::cout << "Base URL: " << base_url << std::endl;

    // Validate the URL format
    std::regex urlFormat(R"(https?://\S+)");
    if (!std::regex_match(url, urlFormat)) {
        std::cerr << "Invalid URL format. Please enter a valid URL starting with http:// or https://" << std::endl;
        return 1;
    }

    ThreadPool* threadPool = new ThreadPool(num_threads);
    std::mutex setMutex;

    if (option_urlset == 0){
        SetList urlSet;
        threadPool->add_task_to_queue([url, base_url, &urlSet, &threadPool, &setMutex]() {
            crawl_parallel(url, base_url, urlSet, *threadPool, setMutex);
        });
        delete threadPool;
        std::cout << "URLs found" << std::endl;
        urlSet.display();
        std::cout << "Number of URLs: " << urlSet.getSize() << std::endl;
    } else if (option_urlset == 1){
        CoarseHashTable<std::string> urlSet(32);
        threadPool->add_task_to_queue([url, base_url, &urlSet, &threadPool, &setMutex]() {
            crawl_parallel(url, base_url, urlSet, *threadPool, setMutex);
        });
        delete threadPool;
        std::cout << "URLs found" << std::endl;
        urlSet.display();
        std::cout << "Number of URLs: " << urlSet.getSize() << std::endl;
    } else if (option_urlset == 2){
        StripedHashTable<std::string> urlSet(32);
        threadPool->add_task_to_queue([url, base_url, &urlSet, &threadPool, &setMutex]() {
            crawl_parallel(url, base_url, urlSet, *threadPool, setMutex);
        });
        delete threadPool;
        std::cout << "URLs found" << std::endl;
        urlSet.display();
        std::cout << "Number of URLs: " << urlSet.getSize() << std::endl;
    } else {
        std::cerr << "Wrong url set option, please use 0 (SetList), 1 (CoarseHashTable) or 2 (StripedHashTable)" << std::endl;
        return 1;
    }

    auto stop = std::chrono::high_resolution_clock::now(); 
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << "Execution time (seconds): ";
	std::cout << duration.count()/1000/1000 << std::endl;

    return 0;
}
